#include "VL53.h"

#define VL53_TIMEOUT_MS  10

static I2C_HandleTypeDef *vl53_i2c;
static uint8_t stop_variable;   // valor interno del sensor que hay que devolverle antes de cada medicion
static uint8_t error_i2c;       // se pone en 1 si falla cualquier comunicacion
static uint8_t paso_calibracion;   // en que paso va VL53_Calibrar()


static void Escribir(uint8_t registro, uint8_t valor)
{
    if (error_i2c) return;
    if (HAL_I2C_Mem_Write(vl53_i2c, VL53_DIRECCION << 1, registro, I2C_MEMADD_SIZE_8BIT,
                          &valor, 1, VL53_TIMEOUT_MS) != HAL_OK) {
        error_i2c = 1;
    }
}

static void EscribirVarios(uint8_t registro, uint8_t *datos, uint16_t cantidad)
{
    if (error_i2c) return;
    if (HAL_I2C_Mem_Write(vl53_i2c, VL53_DIRECCION << 1, registro, I2C_MEMADD_SIZE_8BIT,
                          datos, cantidad, VL53_TIMEOUT_MS) != HAL_OK) {
        error_i2c = 1;
    }
}

static void LeerVarios(uint8_t registro, uint8_t *destino, uint16_t cantidad)
{
    if (error_i2c) return;
    if (HAL_I2C_Mem_Read(vl53_i2c, VL53_DIRECCION << 1, registro, I2C_MEMADD_SIZE_8BIT,
                         destino, cantidad, VL53_TIMEOUT_MS) != HAL_OK) {
        error_i2c = 1;
    }
}

static uint8_t Leer(uint8_t registro)
{
    uint8_t valor = 0;
    LeerVarios(registro, &valor, 1);
    return valor;
}


/* Tabla "DefaultTuningSettings" de la API de ST: pares {registro, valor}.
   El registro 0xFF cambia de "pagina", por eso se repite tanto. */
static const uint8_t ajustes_st[][2] = {
    {0xFF, 0x01}, {0x00, 0x00},
    {0xFF, 0x00}, {0x09, 0x00}, {0x10, 0x00}, {0x11, 0x00},
    {0x24, 0x01}, {0x25, 0xFF}, {0x75, 0x00},
    {0xFF, 0x01}, {0x4E, 0x2C}, {0x48, 0x00}, {0x30, 0x20},
    {0xFF, 0x00}, {0x30, 0x09}, {0x54, 0x00}, {0x31, 0x04}, {0x32, 0x03},
    {0x40, 0x83}, {0x46, 0x25}, {0x60, 0x00}, {0x27, 0x00}, {0x50, 0x06},
    {0x51, 0x00}, {0x52, 0x96}, {0x56, 0x08}, {0x57, 0x30}, {0x61, 0x00},
    {0x62, 0x00}, {0x64, 0x00}, {0x65, 0x00}, {0x66, 0xA0},
    {0xFF, 0x01}, {0x22, 0x32}, {0x47, 0x14}, {0x49, 0xFF}, {0x4A, 0x00},
    {0xFF, 0x00}, {0x7A, 0x0A}, {0x7B, 0x00}, {0x78, 0x21},
    {0xFF, 0x01}, {0x23, 0x34}, {0x42, 0x00}, {0x44, 0xFF}, {0x45, 0x26},
    {0x46, 0x05}, {0x40, 0x40}, {0x0E, 0x06}, {0x20, 0x1A}, {0x43, 0x40},
    {0xFF, 0x00}, {0x34, 0x03}, {0x35, 0x44},
    {0xFF, 0x01}, {0x31, 0x04}, {0x4B, 0x09}, {0x4C, 0x05}, {0x4D, 0x04},
    {0xFF, 0x00}, {0x44, 0x00}, {0x45, 0x20}, {0x47, 0x08}, {0x48, 0x28},
    {0x67, 0x00}, {0x70, 0x04}, {0x71, 0x01}, {0x72, 0xFE}, {0x76, 0x00},
    {0x77, 0x00},
    {0xFF, 0x01}, {0x0D, 0x01},
    {0xFF, 0x00}, {0x80, 0x01}, {0x01, 0xF8},
    {0xFF, 0x01}, {0x8E, 0x01}, {0x00, 0x01},
    {0xFF, 0x00}, {0x80, 0x00},
};


/* Los SPAD son los pixeles que detectan la luz del laser. Cada sensor trae
   grabado de fabrica cuantos SPAD de referencia debe usar y de que tipo.
   Esta funcion lee ese dato de la memoria interna del sensor. */
static uint8_t LeerInfoSpad(uint8_t *cantidad, uint8_t *es_apertura)
{
    Escribir(0x80, 0x01);
    Escribir(0xFF, 0x01);
    Escribir(0x00, 0x00);
    Escribir(0xFF, 0x06);
    Escribir(0x83, Leer(0x83) | 0x04);
    Escribir(0xFF, 0x07);
    Escribir(0x81, 0x01);
    Escribir(0x80, 0x01);
    Escribir(0x94, 0x6B);
    Escribir(0x83, 0x00);

    uint8_t intentos = 0;
    while (Leer(0x83) == 0x00) {
        if (error_i2c || (++intentos > 20)) {
            return 0;
        }
    }

    Escribir(0x83, 0x01);
    uint8_t info = Leer(0x92);
    *cantidad    = info & 0x7F;
    *es_apertura = (info >> 7) & 0x01;

    Escribir(0x81, 0x00);
    Escribir(0xFF, 0x06);
    Escribir(0x83, Leer(0x83) & ~0x04);
    Escribir(0xFF, 0x01);
    Escribir(0x00, 0x01);
    Escribir(0xFF, 0x00);
    Escribir(0x80, 0x00);
    return 1;
}

/* Activa solo los SPAD de referencia que indica la informacion de fabrica. */
static uint8_t ConfigurarSpads(void)
{
    uint8_t cantidad, es_apertura;
    uint8_t mapa[6];   // 48 bits, uno por cada SPAD de referencia

    if (!LeerInfoSpad(&cantidad, &es_apertura)) {
        return 0;
    }
    LeerVarios(VL53_REG_SPAD_ENABLES, mapa, 6);

    Escribir(0xFF, 0x01);
    Escribir(0x4F, 0x00);
    Escribir(0x4E, 0x2C);
    Escribir(0xFF, 0x00);
    Escribir(VL53_REG_SPAD_START_SELECT, 0xB4);

    // Los SPAD de tipo "apertura" empiezan en el bit 12 del mapa
    uint8_t primero = es_apertura ? 12 : 0;
    uint8_t activados = 0;
    for (uint8_t i = 0; i < 48; i++) {
        if (i < primero || activados == cantidad) {
            mapa[i / 8] &= ~(1 << (i % 8));        // apagar este SPAD
        } else if ((mapa[i / 8] >> (i % 8)) & 0x01) {
            activados++;
        }
    }
    EscribirVarios(VL53_REG_SPAD_ENABLES, mapa, 6);
    return 1;
}

/* Verifica que el sensor responda y le carga toda su configuracion. */
uint8_t VL53_Init(I2C_HandleTypeDef *hi2c)
{
    vl53_i2c  = hi2c;
    error_i2c = 0;
    paso_calibracion = 0;

    if (Leer(VL53_REG_MODEL_ID) != VL53_MODEL_ID || error_i2c) {
        return 0;
    }

    Escribir(0x89, Leer(0x89) | 0x01);
    Escribir(VL53_REG_I2C_MODE, 0x00);

    // Guardar el "stop variable" que se usa al iniciar cada medicion
    Escribir(0x80, 0x01);
    Escribir(0xFF, 0x01);
    Escribir(0x00, 0x00);
    stop_variable = Leer(0x91);
    Escribir(0x00, 0x01);
    Escribir(0xFF, 0x00);
    Escribir(0x80, 0x00);

    // Señal minima de retorno para aceptar una medicion: 0.25 MCPS (valor de ST)
    Escribir(VL53_REG_MSRC_CONFIG, Leer(VL53_REG_MSRC_CONFIG) | 0x12);
    uint8_t limite[2] = {0x00, 0x20};
    EscribirVarios(VL53_REG_SIGNAL_RATE_LIMIT, limite, 2);
    Escribir(VL53_REG_SEQUENCE_CONFIG, 0xFF);

    if (!ConfigurarSpads()) {
        return 0;
    }

    for (uint16_t i = 0; i < sizeof(ajustes_st) / sizeof(ajustes_st[0]); i++) {
        Escribir(ajustes_st[i][0], ajustes_st[i][1]);
    }

    // El sensor avisa "medicion lista" en el registro INTERRUPT_STATUS
    Escribir(VL53_REG_INTERRUPT_CONFIG, 0x04);
    Escribir(VL53_REG_GPIO_ACTIVE_HIGH, Leer(VL53_REG_GPIO_ACTIVE_HIGH) & ~0x10);
    Escribir(VL53_REG_INTERRUPT_CLEAR, 0x01);

    Escribir(VL53_REG_SEQUENCE_CONFIG, 0xE8);

    return !error_i2c;
}

uint8_t VL53_Calibrar(void)
{
    error_i2c = 0;

    switch (paso_calibracion) {

        case 0:   // arrancar la calibracion de VHV
            Escribir(VL53_REG_SEQUENCE_CONFIG, 0x01);
            Escribir(VL53_REG_SYSRANGE_START, 0x01 | 0x40);
            paso_calibracion = 1;
            break;

        case 1:   // cuando termine la de VHV, arrancar la de fase
            if (VL53_DatoListo()) {
                Escribir(VL53_REG_INTERRUPT_CLEAR, 0x01);
                Escribir(VL53_REG_SYSRANGE_START, 0x00);
                Escribir(VL53_REG_SEQUENCE_CONFIG, 0x02);
                Escribir(VL53_REG_SYSRANGE_START, 0x01);
                paso_calibracion = 2;
            }
            break;

        default:  // cuando termine la de fase, volver a la secuencia normal de medicion
            if (VL53_DatoListo()) {
                Escribir(VL53_REG_INTERRUPT_CLEAR, 0x01);
                Escribir(VL53_REG_SYSRANGE_START, 0x00);
                Escribir(VL53_REG_SEQUENCE_CONFIG, 0xE8);
                paso_calibracion = 0;
                return error_i2c ? VL53_ERROR : VL53_OK;
            }
            break;
    }

    if (error_i2c) {
        paso_calibracion = 0;
        return VL53_ERROR;
    }
    return VL53_OCUPADO;
}


/* Arranca una sola medicion. No espera a que termine (tarda unos 30 ms). */
uint8_t VL53_IniciarMedicion(void)
{
    error_i2c = 0;

    // Si el sensor perdio la alimentacion olvida su configuracion: se avisa con
    // un 0 para que se vuelva a llamar VL53_Init() y VL53_Calibrar().
    if (Leer(VL53_REG_INTERRUPT_CONFIG) != 0x04) {
        return 0;
    }

    Escribir(0x80, 0x01);
    Escribir(0xFF, 0x01);
    Escribir(0x00, 0x00);
    Escribir(0x91, stop_variable);
    Escribir(0x00, 0x01);
    Escribir(0xFF, 0x00);
    Escribir(0x80, 0x00);

    Escribir(VL53_REG_SYSRANGE_START, 0x01);
    return !error_i2c;
}


/* Regresa 1 si la medicion iniciada ya termino. */
uint8_t VL53_DatoListo(void)
{
    error_i2c = 0;
    return (Leer(VL53_REG_INTERRUPT_STATUS) & 0x07) != 0;
}


/* Lee la distancia en milimetros. Llamar solo cuando VL53_DatoListo() regrese 1. */
uint8_t VL53_LeerDistancia(uint16_t *distancia_mm)
{
    uint8_t d[2] = {0, 0};
    error_i2c = 0;

    LeerVarios(VL53_REG_RESULT_RANGE, d, 2);
    *distancia_mm = ((uint16_t)d[0] << 8) | d[1];

    Escribir(VL53_REG_INTERRUPT_CLEAR, 0x01);   // dejar listo el aviso para la siguiente medición
    return !error_i2c;
}
