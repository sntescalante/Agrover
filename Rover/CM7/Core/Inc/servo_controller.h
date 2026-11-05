#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "main.h"

#define RIGHT 0U
#define LEFT 1U

#define MAX_PULSE 2500U //En microsegundos
#define MIN_PULSE 500U
#define THETA_MAX 180

typedef struct {
    uint8_t front_right;
    uint8_t front_left;
    uint8_t rear_right;
    uint8_t rear_left;
} SteeringAngles;


uint8_t ServoMotors_PWM_Init(TIM_HandleTypeDef * htim);

SteeringAngles ChangeRoverDirection(TIM_HandleTypeDef * htim, TIM_OC_InitTypeDef * sConfig,
							SteeringAngles current_angles, uint8_t direction);

SteeringAngles MoveRight(TIM_OC_InitTypeDef * sConfig, TIM_HandleTypeDef * htim,
							SteeringAngles current_angles);

SteeringAngles MoveLeft(TIM_OC_InitTypeDef * sConfig, TIM_HandleTypeDef * htim,
							SteeringAngles current_angles);

#endif




