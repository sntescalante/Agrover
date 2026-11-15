#ifndef RTOS_ROVER_H
#define RTOS_ROVER_H

#include <stdint.h>
#include "cmsis_os2.h"
#include "main.h"

extern osThreadId_t measureEnvironmentSensorsHandle;
extern osThreadId_t processEnvironmentDataHandle;
extern osThreadId_t sendDataHandle;
extern osThreadId_t receiveDataHandle;
extern osThreadId_t measureSoilSensorHandle;

extern osThreadAttr_t measureEnvironmentSensorsAttr;
extern osThreadAttr_t processEnvironmentDataAttr;
extern osThreadAttr_t sendDataAttr;
extern osThreadAttr_t receiveDataAttr;
extern osThreadAttr_t measureSoilSensorAttr;

extern osMutexId_t uartMutexID;
extern const osMutexAttr_t uartMutexAttr;

extern osMessageQueueId_t measureQueueHandle;
extern const osMessageQueueAttr_t medQueue_attr;

extern osSemaphoreId_t soilMeasureSemHandle;
extern const osSemaphoreAttr_t btnSem_attr;


void measureEnvironmentSensorsTask(void * argument);
void processEnvironmentDataTask(void * argument);
void sendDataHandle(void * argument);
void receiveDataTask(void * argument);
void measureSoilSensorTask(void * argument);

#endif
