#include "RTOS_Rover.h"


//Attributes y IDs de Tasks

osThreadId_t sendDataHandle;

const osThreadAttr_t sendDataAttr = {
		.name = "SendData",
		.stack_size = 512*4,
		.priority = (osPriority_t) osPriorityBelowNormal
};


osThreadId_t receiveDataHandle;

const osThreadAttr_t receiveDataAttr = {
		.name = "ReceiveData",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityHigh
};

osThreadId_t processWebCommandHandle;

const osThreadAttr_t processWebCommandAttr = {
		.name = "ProcessWebCommand",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t measureDistanceHandle;

const osThreadAttr_t measureDistanceAttr = {
		.name = "MeasureDistance",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t processDistanceHandle;

const osThreadAttr_t processDistanceAttr = {
		.name = "ProcessDistance",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t measureSoilHandle;

const osThreadAttr_t measureSoilAttr = {
		.name = "MeasureSoil",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t processSoilDataHandle;

const osThreadAttr_t proccessSoilDataAttr = {
		.name = "processSoilData",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t wateringHandle;

const osThreadAttr_t wateringAttr = {
		.name = "Watering",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t measureEnvironmentHandle;

const osThreadAttr_t measureEnvironmentAttr = {
		.name = "MeasureEnvironment",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityLow
};

osThreadId_t processEnvironmentDataHandle;

const osThreadAttr_t processEnvironmentDataAttr = {
		.name = "ProcessEnvironmentData",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityLow
};

//Mutex para USART3 (Terminal)

osMutexId_t uartMutexHandle;
const osMutexAttr_t uartMutexAttr= { .name = "uartMutex" };

//Mutex para I2C2

osMutexId_t i2c2MutexHandle;
const osMutexAttr_t i2c2MutexAttr = {.name = "I2C2Mutex"};

//Queue para 5 mediciones de sensor de suelo

osMessageQueueId_t soilMeasureQueueHandle;
const osMessageQueueAttr_t soilMedQueue_attr = { .name = "soilMedQueue" };

//Semaphore para habilitar task de recepción de comando

osSemaphoreId_t receiveDataSemHandle;
const osSemaphoreAttr_t receiveDataSemAttr = { .name = "receiveDataSem"};

//Semaphore para habilitar task de procesar comando

osSemaphoreId_t processDataSemHandle;
const osSemaphoreAttr_t processDataSemAttr = { .name = "processDataSem"};

//Semaphore para habilitar task de medicion de suelo

osSemaphoreId_t soilMeasureSemHandle;
const osSemaphoreAttr_t soilMeasureSemAttr = { .name = "soilMeasureSem"};

//Semaphore para habilitar task de procesar datos de mediciones de suelo

osSemaphoreId_t soilDataProcessSemHandle;
const osSemaphoreAttr_t soilDataProcessSemAttr = {.name = "soilDataProcessSem"};

//Semaphore para habilitar task de lectura de distancia

osSemaphoreId_t distanceMeasureSemHandle;
const osSemaphoreAttr_t distanceMeasureSemAttr = { .name = "distanceMeasureSem"};

//Semaphore para habilitar task de procesamiento de distancia

osSemaphoreId_t distanceProcessSemHandle;
const osSemaphoreAttr_t distanceProcessSemAttr = { .name = "distanceProcessSem"};

//Semaphore para habilitar task de riego

osSemaphoreId_t wateringSemHandle;
const osSemaphoreAttr_t wateringProcessSemAttr = { .name = "wateringSem"};


osSemaphoreId_t transmitDataSemHandle;
const osSemaphoreAttr_t transmitDataSemAttr = {.name = "transmitSem"};


osSemaphoreId_t guvaProcessSemHandle;
const osSemaphoreAttr_t guvaProcessSemAttr = {.name = "guvaProcessSem"};

