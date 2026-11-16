#ifndef TRANSMITDATA_H
#define TRANSMITDATA_H

#include "main.h"
#include "RTOS_Rover.h"
#include "sensor_ultravioleta.h"
#include "BME280.h"
#include "soil_sensor.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    uint16_t dataLength;
    uint16_t index;
} TransmitWifiDataConfig;

void readEnvironmentSensorsTask(void *argument);
void processGUVAsensorTask(void *argument);
void TransmitDataTask(void *argument);

#endif
