#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include "main.h"

#define SOIL_UART huart2

typedef struct {
	float humedad; //%
	float temperatura; //°C
	uint16_t conductividad; //us/cm
	float ph;
	uint16_t nitrogeno; //mg/kg
	uint16_t fosforo;
	uint16_t potasio;
} SoilData;

typedef enum {
	SOIL_OK = 0,
	SOIL_TIMEOUT,
	SOIL_CRC_ERROR
} SoilStatus;

SoilStatus Soil_Read(SoilData * data);

#endif
