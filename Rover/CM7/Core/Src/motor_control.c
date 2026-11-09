#include "motor_control.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern uint8_t dc_motors_forward_duty_cycle;
extern uint8_t dc_motors_backwards_duty_cycle;

extern const uint16_t ARR;

uint16_t pulse;
uint8_t current_direction = STOPPED;

uint8_t DCmotorsInit(){
	dc_motors_forward_duty_cycle = 0;
	dc_motors_backwards_duty_cycle = 0;

	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK) {
		return 1;
	}

	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK) {
		return 1;
	}

	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK) {
		return 1;
	}

	if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4) != HAL_OK) {
		return 1;
	}

	if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) != HAL_OK) {
		return 1;
	}

	if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2) != HAL_OK) {
		return 1;
	}

	return 0;
}

uint16_t dutyCycleToPulse(uint8_t dc_motors_duty_cycle) {
	return (uint16_t)(((uint32_t)dc_motors_duty_cycle * (ARR + 1)) / 100);
}

uint8_t MoveForward() {

	dc_motors_backwards_duty_cycle = 0;

	if (dc_motors_forward_duty_cycle <= 90) {
		dc_motors_forward_duty_cycle += 10;
	} else {
		dc_motors_forward_duty_cycle = 100;
	}

	pulse = dutyCycleToPulse(dc_motors_forward_duty_cycle);

	//Si ya se estaba moviendo hacia adelante
	if (current_direction == FORWARD) {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);
	} else {
	//Si estaba parado o en reversa

		//PWM con duty cycle de 0%
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);

		//Apagar pines de retroceso

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_15, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

		HAL_Delay(20);

		//Activar pines de avance
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

		//Activar PWM

		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);

	}

	current_direction = FORWARD;
	return 0;
}
uint8_t MoveBackwards() {
	dc_motors_forward_duty_cycle = 0;

	if (dc_motors_backwards_duty_cycle <= 90) {
		dc_motors_backwards_duty_cycle += 10;
	} else {
		dc_motors_backwards_duty_cycle = 100;
	}

	pulse = dutyCycleToPulse(dc_motors_backwards_duty_cycle);

	//Si ya se estaba moviendo hacia adelante
	if (current_direction == BACKWARDS) {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);
	} else {
	//Si estaba parado o en reversa

		//PWM con duty cycle de 0%
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);

		//Apagar pines de avance

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

		HAL_Delay(20);

		//Activar pines de retroceso
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_15, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);

		//Activar PWM

		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pulse);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);

	}

	current_direction = BACKWARDS;

	return 0;
}
uint8_t Stop() {
	dc_motors_backwards_duty_cycle = 0;
	dc_motors_forward_duty_cycle = 0;
	current_direction = STOPPED;
	pulse = 0;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_15, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);

	return 0;
}

