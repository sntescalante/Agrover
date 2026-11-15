#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include "main.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include <stdbool.h>
#include "servo_controller.h"

#define BUFFER_MAX_UART_RECEIVE 8

typedef struct {
    uint8_t buffer[BUFFER_MAX_UART_RECEIVE];
    uint16_t dataLength;
    uint16_t index;
} ReceiveWifiDataConfig;

void receiveDataTask(void * argument);
void processWebDataTask(void * argument);
void measureDistanceTask(void * argument);
void processDistanceTask(void * argument);

#endif

