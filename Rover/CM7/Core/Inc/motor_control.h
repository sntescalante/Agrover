#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "main.h"
#include <math.h>

#define PULSE_RPM_0 0

#define STOPPED   0
#define FORWARD   1
#define BACKWARDS 2

uint8_t DCmotorsInit();
uint8_t MoveForward();
uint8_t MoveBackwards();
uint8_t Stop();
uint16_t dutyCycleToPulse(uint8_t dc_motors_duty_cycle);

#endif
