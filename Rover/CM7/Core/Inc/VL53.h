/*
 * VL53.h
 * Driver sencillo para el sensor de distancia laser VL53L0X por I2C.
 *
 * Ninguna funcion se queda esperando al sensor. La espera la decide quien las
 * llama (un timer en el main, o un osDelay en un task de RTOS).
 *
 * Orden de uso:
 *   1. VL53_Init()     -> verifica el sensor y le carga su configuracion
 *   2. VL53_Calibrar() -> llamarla periodicamente (ej. cada 10 ms) mientras
 *                         regrese VL53_OCUPADO. Termina en unos 40 ms.
 *   3. Para cada medicion:
 *        VL53_IniciarMedicion()  -> arranca la medicion (tarda ~30 ms)
 *        VL53_DatoListo()        -> preguntar periodicamente hasta que regrese 1
 *        VL53_LeerDistancia()    -> lee el resultado en milimetros
 *
 * Las funciones regresan VL53_OK (1) si salio bien y VL53_ERROR (0) si fallo.
 */

#ifndef INC_VL53_H_
#define INC_VL53_H_

#include "stm32h7xx_hal.h"

#define VL53_DIRECCION        0x29
#define VL53_MODEL_ID         0xEE   // valor fijo que regresa el registro de identificacion

/* Registros con nombre (ST no publica el mapa completo, estos son los de su API) */
#define VL53_REG_SYSRANGE_START       0x00   // arranca una medicion
#define VL53_REG_SEQUENCE_CONFIG      0x01   // que pasos hace el sensor en cada medicion
#define VL53_REG_INTERRUPT_CONFIG     0x0A
#define VL53_REG_INTERRUPT_CLEAR      0x0B
#define VL53_REG_INTERRUPT_STATUS     0x13   // bits 0-2 distintos de 0 = medicion lista
#define VL53_REG_RESULT_RANGE         0x1E   // distancia en mm (2 bytes, primero el alto)
#define VL53_REG_SIGNAL_RATE_LIMIT    0x44
#define VL53_REG_MSRC_CONFIG          0x60
#define VL53_REG_GPIO_ACTIVE_HIGH     0x84
#define VL53_REG_I2C_MODE             0x88
#define VL53_REG_SPAD_ENABLES         0xB0   // 6 bytes
#define VL53_REG_SPAD_START_SELECT    0xB6
#define VL53_REG_MODEL_ID             0xC0

// El sensor regresa 8190 u 8191 mm cuando no ve ningun objeto (alcance real ~1.2 m)
#define VL53_FUERA_DE_RANGO   8190

/* Valores que regresan las funciones */
#define VL53_ERROR    0
#define VL53_OK       1
#define VL53_OCUPADO  2   // solo VL53_Calibrar(): todavia no termina, volver a llamarla

uint8_t VL53_Init(I2C_HandleTypeDef *hi2c);
uint8_t VL53_Calibrar(void);

uint8_t VL53_IniciarMedicion(void);
uint8_t VL53_DatoListo(void);
uint8_t VL53_LeerDistancia(uint16_t *distancia_mm);

#endif /* INC_VL53_H_ */
