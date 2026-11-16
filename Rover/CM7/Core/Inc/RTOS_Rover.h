#ifndef RTOS_ROVER_H
#define RTOS_ROVER_H

#include <stdint.h>
#include "cmsis_os2.h"
#include "main.h"

#define N_SOIL_MEASURMENTES 5

// Threads
extern osThreadId_t sendDataHandle;
extern const osThreadAttr_t sendDataAttr;

extern osThreadId_t receiveDataHandle;
extern const osThreadAttr_t receiveDataAttr;

extern osThreadId_t processWebCommandHandle;
extern const osThreadAttr_t processWebCommandAttr;

extern osThreadId_t measureDistanceHandle;
extern const osThreadAttr_t measureDistanceAttr;

extern osThreadId_t processDistanceHandle;
extern const osThreadAttr_t processDistanceAttr;

extern osThreadId_t measureSoilHandle;
extern const osThreadAttr_t measureSoilAttr;

extern osThreadId_t processSoilDataHandle;
extern const osThreadAttr_t proccessSoilDataAttr;

extern osThreadId_t wateringHandle;
extern const osThreadAttr_t wateringAttr;

extern osThreadId_t measureEnvironmentHandle;
extern const osThreadAttr_t measureEnvironmentAttr;

extern osThreadId_t processEnvironmentDataHandle;
extern const osThreadAttr_t processEnvironmentDataAttr;


// Mutex
extern osMutexId_t uartMutexHandle;
extern const osMutexAttr_t uartMutexAttr;

extern osMutexId_t i2c2MutexHandle;
extern const osMutexAttr_t i2c2MutexAttr;


// Queue
extern osMessageQueueId_t soilMeasureQueueHandle;
extern const osMessageQueueAttr_t soilMedQueue_attr;


// Semaphores
extern osSemaphoreId_t receiveDataSemHandle;
extern const osSemaphoreAttr_t receiveDataSemAttr;

extern osSemaphoreId_t processDataSemHandle;
extern const osSemaphoreAttr_t processDataSemAttr;

extern osSemaphoreId_t soilMeasureSemHandle;
extern const osSemaphoreAttr_t soilMeasureSemAttr;

extern osSemaphoreId_t soilDataProcessSemHandle;
extern const osSemaphoreAttr_t soilDataProcessSemAttr;

extern osSemaphoreId_t distanceMeasureSemHandle;
extern const osSemaphoreAttr_t distanceMeasureSemAttr;

extern osSemaphoreId_t distanceProcessSemHandle;
extern const osSemaphoreAttr_t distanceProcessSemAttr;

extern osSemaphoreId_t guvaProcessSemHandle;
extern const osSemaphoreAttr_t guvaProcessSemAttr;

extern osSemaphoreId_t transmitDataSemHandle;
extern const osSemaphoreAttr_t transmitDataSemAttr;

extern osSemaphoreId_t wateringSemHandle;
extern const osSemaphoreAttr_t wateringProcessSemAttr;


#endif
