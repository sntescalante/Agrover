#include "BME280.h"

#define BME280_TIMEOUT_MS  10

/* Coeficientes de calibracion que cada sensor trae grabados de fabrica */
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4, dig_H5;
    int8_t   dig_H6;
} BME280_Calib;

static I2C_HandleTypeDef *bme_i2c;
static uint8_t            bme_direccion; // 0x76 o 0x77, se detecta en BME280_Init()
static BME280_Calib       calib;
static int32_t            t_fine;


uint8_t BME280_LeerRegistro(uint8_t registro, uint8_t *destino, uint16_t cantidad)
{
    return HAL_I2C_Mem_Read(bme_i2c, bme_direccion << 1, registro, I2C_MEMADD_SIZE_8BIT,
                            destino, cantidad, BME280_TIMEOUT_MS) == HAL_OK;
}

uint8_t BME280_EscribirRegistro(uint8_t registro, uint8_t valor)
{
    return HAL_I2C_Mem_Write(bme_i2c, bme_direccion << 1, registro, I2C_MEMADD_SIZE_8BIT,
                             &valor, 1, BME280_TIMEOUT_MS) == HAL_OK;
}


/* Detecta el sensor y lo deja midiendo continuamente (modo normal). No tiene esperas. */
uint8_t BME280_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t id = 0;

    bme_i2c = hi2c;

    // El sensor puede estar en 0x76 o 0x77 segun como este conectado su pin SDO
    if (HAL_I2C_IsDeviceReady(bme_i2c, 0x76 << 1, 1, BME280_TIMEOUT_MS) == HAL_OK) {
        bme_direccion = 0x76;
    } else if (HAL_I2C_IsDeviceReady(bme_i2c, 0x77 << 1, 1, BME280_TIMEOUT_MS) == HAL_OK) {
        bme_direccion = 0x77;
    } else {
        return 0;
    }

    if (!BME280_LeerRegistro(BME280_REG_ID, &id, 1) || id != BME280_CHIP_ID) {
        return 0;
    }

    // Primero se pasa a modo sleep: el registro config solo se puede cambiar asi.
    // Despues el orden importa: ctrl_hum solo se aplica cuando se escribe ctrl_meas.
    return BME280_EscribirRegistro(BME280_REG_CTRL_MEAS, BME280_MODO_SLEEP)   &&
           BME280_EscribirRegistro(BME280_REG_CTRL_HUM,  BME280_CTRL_HUM_VAL) &&
           BME280_EscribirRegistro(BME280_REG_CONFIG,    BME280_CONFIG_VAL)   &&
           BME280_EscribirRegistro(BME280_REG_CTRL_MEAS, BME280_CTRL_MEAS_VAL);
}


/* Lee los coeficientes de calibracion y los guarda para usarlos en las formulas. */
uint8_t BME280_Calibrar(void)
{
    uint8_t c1[26];   // registros 0x88 a 0xA1
    uint8_t c2[7];    // registros 0xE1 a 0xE7

    if (!BME280_LeerRegistro(BME280_REG_CALIB1, c1, 26) ||
        !BME280_LeerRegistro(BME280_REG_CALIB2, c2, 7)) {
        return 0;
    }

    calib.dig_T1 = (uint16_t)((c1[1]  << 8) | c1[0]);
    calib.dig_T2 = (int16_t) ((c1[3]  << 8) | c1[2]);
    calib.dig_T3 = (int16_t) ((c1[5]  << 8) | c1[4]);

    calib.dig_P1 = (uint16_t)((c1[7]  << 8) | c1[6]);
    calib.dig_P2 = (int16_t) ((c1[9]  << 8) | c1[8]);
    calib.dig_P3 = (int16_t) ((c1[11] << 8) | c1[10]);
    calib.dig_P4 = (int16_t) ((c1[13] << 8) | c1[12]);
    calib.dig_P5 = (int16_t) ((c1[15] << 8) | c1[14]);
    calib.dig_P6 = (int16_t) ((c1[17] << 8) | c1[16]);
    calib.dig_P7 = (int16_t) ((c1[19] << 8) | c1[18]);
    calib.dig_P8 = (int16_t) ((c1[21] << 8) | c1[20]);
    calib.dig_P9 = (int16_t) ((c1[23] << 8) | c1[22]);

    calib.dig_H1 = c1[25];
    calib.dig_H2 = (int16_t)((c2[1] << 8) | c2[0]);
    calib.dig_H3 = c2[2];
    // H4 y H5 son de 12 bits y comparten el registro 0xE5 (c2[4])
    calib.dig_H4 = (int16_t)(((int8_t)c2[3] * 16) | (c2[4] & 0x0F));
    calib.dig_H5 = (int16_t)(((int8_t)c2[5] * 16) | (c2[4] >> 4));
    calib.dig_H6 = (int8_t)c2[6];

    return 1;
}


/* Formula de la datasheet. Regresa centesimas de grado C y actualiza t_fine. */
static int32_t Compensar_T(int32_t adc_T)
{
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) * ((int32_t)calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

/* Formula de la datasheet (version de 64 bits). Regresa Pascales x 256. */
static uint32_t Compensar_P(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;
    if (var1 == 0) {
        return 0;   // evita dividir entre cero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    return (uint32_t)p;
}

/* Formula de la datasheet. Regresa %RH x 1024. */
static uint32_t Compensar_H(int32_t adc_H)
{
    int32_t v = t_fine - ((int32_t)76800);
    v = (((((adc_H << 14) - (((int32_t)calib.dig_H4) << 20) - (((int32_t)calib.dig_H5) * v)) +
           ((int32_t)16384)) >> 15) *
         (((((((v * ((int32_t)calib.dig_H6)) >> 10) *
              (((v * ((int32_t)calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
            ((int32_t)2097152)) * ((int32_t)calib.dig_H2) + 8192) >> 14));
    v = v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)calib.dig_H1)) >> 4);
    if (v < 0) v = 0;
    if (v > 419430400) v = 419430400;
    return (uint32_t)(v >> 12);
}


/* Lee la ultima medicion del sensor y la convierte a unidades reales. */
uint8_t BME280_Leer(BME280_Datos *datos)
{
    uint8_t d[8];
    uint8_t modo = 0;

    // Si el sensor perdio la alimentacion vuelve a modo sleep y olvida su configuracion.
    // Se revisa aqui para avisar con un 0 y que se vuelva a llamar BME280_Init().
    if (!BME280_LeerRegistro(BME280_REG_CTRL_MEAS, &modo, 1) || modo != BME280_CTRL_MEAS_VAL) {
        return 0;
    }

    if (!BME280_LeerRegistro(BME280_REG_DATOS, d, 8)) {
        return 0;
    }

    int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);
    int32_t adc_H = ((int32_t)d[6] << 8)  | d[7];

    datos->temperatura = Compensar_T(adc_T);
    datos->presion     = Compensar_P(adc_P) / 256;
    datos->humedad     = (Compensar_H(adc_H) * 100) / 1024;

    return 1;
}
