#ifndef SENSOR_ULTRAVIOLETA_H
#define SENSOR_ULTRAVIOLETA_H

#include "main.h"
#include <stdbool.h>

#define UV_HADC hadc1
#define UV_VREF 3.3f
#define UV_ADC_MAX 65535.0f //16 bits
#define UV_V_PINDEX 0.1f //V por punto de índice UV
#define UV_OFFSET_V 0.0f //checar tapandolo
#define UV_NMUESTRAS 32U
#define UV_ADC_TIMEOUT 10U //10ms

#define UV_UMBRAL_BAJO 3.0f
#define UV_UMBRAL_MODERADO 6.0f
#define UV_UMBRAL_ALTO 8.0f
#define UV_UMBRAL_MUY_ALTO 11.0f


typedef enum {
	UV_OK = 0,
	UV_ERROR_CALIB,
	UV_ERROR_ADC
} UV_Status_t;

typedef enum {
    UV_RIESGO_BAJO = 0,  //<3
    UV_RIESGO_MODERADO,  //<6
    UV_RIESGO_ALTO,      //<8
    UV_RIESGO_MUY_ALTO,  //<11
    UV_RIESGO_EXTREMO    //11+
} UV_Riesgo_t;

typedef struct {
	bool init;
	UV_Status_t estado;
	uint16_t adc_prom;
	float voltaje;
	float index;
	UV_Riesgo_t riesgo;
}UV_Resultados_t;

extern UV_Resultados_t uv;

UV_Status_t UV_new(void);

#endif
