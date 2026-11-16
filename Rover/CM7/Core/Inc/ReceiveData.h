#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include "main.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include <stdbool.h>
#include "servo_controller.h"
#include "motor_control.h"
#include "soil_sensor.h"
#include "RTOS_Rover.h"
#include "VL53.h"

#define BUFFER_MAX_UART_RECEIVE 8

typedef struct {
    uint16_t dataLength;
    uint16_t index;
} ReceiveWifiDataConfig;

void receiveDataTask(void * argument);
void processWebDataTask(void * argument);
void measureDistanceTask(void * argument);
void processDistanceTask(void * argument);
void soilMeasuringTask(void * argument);
void soilDataProcessTask(void * argument);
void wateringTask(void * argument);

#endif

