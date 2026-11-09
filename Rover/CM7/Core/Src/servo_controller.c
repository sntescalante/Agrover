#include "servo_controller.h"


static const float W  = 20.0f;   // ancho entre ruedas izq-der
static const float Lf = 20.0f;   // eje delantero al centro
static const float Lr = 20.0f;   // eje trasero al centro


uint16_t AngleToPulse(float angle) {
	return (uint16_t)(MIN_PULSE_0 + (angle / 180.0f) * (MAX_PULSE_180 - MIN_PULSE_0));
}

uint8_t ServoMotors_PWM_Init(TIM_HandleTypeDef * htim) {

	//TIM 1 PARA SERVOS DE DIRECCIÓN
	//TIM 8 PARA SERVOS PARA SENSOR DE SUELO

	if (htim->Instance == TIM1) {
		if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1) != HAL_OK) {
			return 1;
		}

		if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2) != HAL_OK) {
			return 1;
		}

		if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3) != HAL_OK) {
			return 1;
		}

		if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_4) != HAL_OK) {
			return 1;
		}
	} else if (htim ->Instance == TIM8) {
		if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1) != HAL_OK) {
			return 1;
		}

		if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2) != HAL_OK) {
			return 1;
		}

	}

	return 0;

}

//Va a ser un cambio de 10° cada 110ms para prueba 1

SteeringAngles ChangeRoverDirection(TIM_HandleTypeDef * htim, TIM_OC_InitTypeDef * sConfig,
							SteeringAngles current_angles, uint8_t direction) {

	SteeringAngles new_angle_steering_servos;

	sConfig->OCMode = TIM_OCMODE_PWM1;
	sConfig->OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig->OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfig->OCFastMode = TIM_OCFAST_DISABLE;
	sConfig->OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfig->OCNIdleState = TIM_OCNIDLESTATE_RESET;

	if (direction == RIGHT) {
		new_angle_steering_servos = MoveRight(sConfig, htim, current_angles);
	} else if (direction == LEFT) {
		new_angle_steering_servos = MoveLeft(sConfig, htim, current_angles);
	} else {
		Error_Handler();
	}

	return new_angle_steering_servos;
}

SteeringAngles MoveRight(TIM_OC_InitTypeDef * sConfig, TIM_HandleTypeDef * htim,
	SteeringAngles current_angles) {


	SteeringAngles new_angles = {
			.front_right = 0.0f, //Inner (ANGULO BASE PARA GIRO DERECHA)
			.front_left = 0.0f, //Exterior
			.rear_right = 0.0f, //Inner
			.rear_left = 0.0f //Exterior
	};

	if (current_angles.front_right >= (THETA_MIN + 10)) {

			new_angles.front_right = current_angles.front_right - 10;

			float theta_di = THETA_CENTER - new_angles.front_right;

			if (theta_di > 0.5f) { // Para que no haya divisiones entre 0

				//Fórmulas de Ackermann

				float d = Lf / tanf(theta_di * DEG2RAD);

				float theta_de = atanf(Lf / (d + W)) * RAD2DEG;  // delantera externa
				float theta_ti = atanf(Lr / d)       * RAD2DEG;  // trasera interna
				float theta_te = atanf(Lr / (d + W)) * RAD2DEG;  // trasera externa

				//Traseras en sentido contrario  a delanteras
				new_angles.front_left = THETA_CENTER - theta_de;
				new_angles.rear_right = THETA_CENTER + theta_ti;
				new_angles.rear_left  = THETA_CENTER + theta_te;

			} else {
				new_angles.front_left = THETA_CENTER;
				new_angles.rear_right = THETA_CENTER;
				new_angles.rear_left  = THETA_CENTER;
			}

			new_angles.front_left = fminf(fmaxf(new_angles.front_left, THETA_MIN), THETA_MAX);
			new_angles.rear_right = fminf(fmaxf(new_angles.rear_right, THETA_MIN), THETA_MAX);
			new_angles.rear_left  = fminf(fmaxf(new_angles.rear_left,  THETA_MIN), THETA_MAX);

		} else {
			new_angles.front_right = current_angles.front_right;
			new_angles.front_left = current_angles.front_left;
			new_angles.rear_right = current_angles.rear_right;
			new_angles.rear_left = current_angles.rear_left;
		}

	uint16_t new_front_inner_pulse = AngleToPulse(new_angles.front_right);
	uint16_t new_front_exterior_pulse = AngleToPulse(new_angles.front_left);
	uint16_t new_rear_inner_pulse = AngleToPulse(new_angles.rear_right);
	uint16_t new_rear_exterior_pulse = AngleToPulse(new_angles.rear_left);


	sConfig -> Pulse = new_front_inner_pulse;

	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_2) != HAL_OK) {
		  Error_Handler();
	}

	sConfig -> Pulse = new_front_exterior_pulse;

	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_1) != HAL_OK) {
	  Error_Handler();
	}

	sConfig -> Pulse = new_rear_inner_pulse;

	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_4) != HAL_OK) {
			Error_Handler();
	}

	sConfig -> Pulse = new_rear_exterior_pulse;

	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}


	return new_angles;
}

SteeringAngles MoveLeft(TIM_OC_InitTypeDef * sConfig, TIM_HandleTypeDef * htim,
	SteeringAngles current_angles) {

	//TEMPORAL PARA PRUEBAS, SAO TIENE UNA FORMULA

	SteeringAngles new_angles = {
				.front_right = 0.0f, //Exterior
				.front_left = 0.0f, //Inner (ANGULO BASE PARA GIRO IZQUIERDA)
				.rear_right = 0.0f, //Exterior
				.rear_left = 0.0f //Inner
	};

	if (current_angles.front_left <= (THETA_MAX - 10)) {

		new_angles.front_left = current_angles.front_left + 10;

		float theta_di = new_angles.front_left - THETA_CENTER;

		if (theta_di > 0.5f) { // Para que no haya divisiones entre 0

			//Fórmulas de Ackermann

			float d = Lf / tanf(theta_di * DEG2RAD);

			float theta_de = atanf(Lf / (d + W)) * RAD2DEG;  // delantera externa
			float theta_ti = atanf(Lr / d)       * RAD2DEG;  // trasera interna
			float theta_te = atanf(Lr / (d + W)) * RAD2DEG;  // trasera externa

			//Traseras en sentido contrario  a delanteras
			new_angles.front_right = THETA_CENTER + theta_de;
			new_angles.rear_left = THETA_CENTER - theta_ti;
			new_angles.rear_right  = THETA_CENTER - theta_te;

		} else {
			new_angles.front_right = THETA_CENTER;
			new_angles.rear_left = THETA_CENTER;
			new_angles.rear_right  = THETA_CENTER;
		}

		new_angles.front_right = fminf(fmaxf(new_angles.front_right, THETA_MIN), THETA_MAX);
		new_angles.rear_left = fminf(fmaxf(new_angles.rear_left, THETA_MIN), THETA_MAX);
		new_angles.rear_right  = fminf(fmaxf(new_angles.rear_right,  THETA_MIN), THETA_MAX);

	} else {
		new_angles.front_right = current_angles.front_right;
		new_angles.front_left = current_angles.front_left;
		new_angles.rear_right = current_angles.rear_right;
		new_angles.rear_left = current_angles.rear_left;
	}


	uint16_t new_front_inner_pulse = AngleToPulse(new_angles.front_right);
	uint16_t new_front_exterior_pulse = AngleToPulse(new_angles.front_left);
	uint16_t new_rear_inner_pulse = AngleToPulse(new_angles.rear_right);
	uint16_t new_rear_exterior_pulse = AngleToPulse(new_angles.rear_left);

	sConfig->Pulse = new_front_inner_pulse;
	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}

	sConfig->Pulse = new_front_exterior_pulse;
	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}

	sConfig->Pulse = new_rear_inner_pulse;
	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_4) != HAL_OK) {
		Error_Handler();
	}

	sConfig->Pulse = new_rear_exterior_pulse;
	if (HAL_TIM_PWM_ConfigChannel(htim, sConfig, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}

	return new_angles;
}
