/*
 * BME280.h
 * Driver sencillo para el sensor BME280 (temperatura, presion y humedad) por I2C.
 *
 * Orden de uso:
 *   1. BME280_Init()     -> detecta el sensor y lo configura
 *   2. BME280_Calibrar() -> lee los coeficientes de calibracion de fabrica
 *   3. BME280_Leer()     -> se llama cada vez que se quiera una medicion
 *
 * Todas las funciones regresan 1 si salio bien y 0 si hubo error.
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_

#include "stm32h7xx_hal.h"

/* Registros del sensor (datasheet Bosch, seccion 5.3) */
#define BME280_REG_CALIB1     0x88   // 26 bytes: dig_T1..dig_P9 y dig_H1
#define BME280_REG_ID         0xD0
#define BME280_REG_RESET      0xE0
#define BME280_REG_CALIB2     0xE1   // 7 bytes: dig_H2..dig_H6
#define BME280_REG_CTRL_HUM   0xF2
#define BME280_REG_STATUS     0xF3
#define BME280_REG_CTRL_MEAS  0xF4
#define BME280_REG_CONFIG     0xF5
#define BME280_REG_DATOS      0xF7   // 8 bytes: presion(3) temperatura(3) humedad(2)

#define BME280_CHIP_ID        0x60
#define BME280_MODO_SLEEP     0x00

/* Configuracion usada por BME280_Init() */
#define BME280_CTRL_HUM_VAL   0x01   // humedad x1
#define BME280_CONFIG_VAL     0x40   // espera de 125 ms entre mediciones, filtro apagado
#define BME280_CTRL_MEAS_VAL  0x27   // temperatura x1, presion x1, modo normal (mide solo)

/* Resultado de una medicion, en enteros para no depender de printf con float */
typedef struct {
    int32_t  temperatura;   // centesimas de grado C   (2534  = 25.34 C)
    uint32_t presion;       // Pascales                (101325 = 1013.25 hPa)
    uint32_t humedad;       // centesimas de %RH       (4756  = 47.56 %)
} BME280_Datos;

uint8_t BME280_Init(I2C_HandleTypeDef *hi2c);
uint8_t BME280_Calibrar(void);
uint8_t BME280_Leer(BME280_Datos *datos);

/* Acceso directo a registros, por si se necesita cambiar la configuracion */
uint8_t BME280_LeerRegistro(uint8_t registro, uint8_t *destino, uint16_t cantidad);
uint8_t BME280_EscribirRegistro(uint8_t registro, uint8_t valor);

#endif /* INC_BME280_H_ */
