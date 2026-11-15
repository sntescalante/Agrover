#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "main.h"
#include <math.h>

#define RIGHT 0U
#define LEFT 1U

#define MAX_PULSE_180 2500U //En microsegundos
#define MIN_PULSE_0 500U
#define THETA_MAX (90 + 40)
#define THETA_CENTER 90
#define THETA_MIN (90 - 40)

#define STEP_DEG 10.0f //Cambio de dirección por cada comando

#define DEG2RAD  0.01745329f //pi / 180
#define RAD2DEG  57.2957795f // 180 / pi


typedef struct {
    float front_right;
    float front_left;
    float rear_right;
    float rear_left;
} SteeringAngles;


uint8_t ServoMotors_PWM_Init(TIM_HandleTypeDef * htim);

uint16_t AngleToPulse(float angle);

SteeringAngles ChangeRoverDirection(TIM_HandleTypeDef * htim,
							SteeringAngles current_angles, uint8_t direction);

SteeringAngles MoveRight(TIM_HandleTypeDef * htim,
							SteeringAngles current_angles, float step);

SteeringAngles MoveLeft(TIM_HandleTypeDef * htim,
							SteeringAngles current_angles, float step);


#endif
