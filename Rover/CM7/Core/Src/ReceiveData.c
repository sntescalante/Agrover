#include "ReceiveData.h"
#include "RTOS_Rover.h"

extern UART_HandleTypeDef huart1;

extern ReceiveWifiDataConfig configWifi;

uint8_t rxByte;
uint8_t webCommand;

//FLAGS DE SUELO Y DISTANCIA GLOBALES
extern volatile bool measuringSoil;
extern volatile bool acceptableDistance;

//Flag para iniciar medición de suelo
bool volatile startSoilMeasuting = false;

//Timers PWM Motores
extern TIM_HandleTypeDef htim1;

//Angulos servos
extern SteeringAngles steering_angles;

void receiveDataTask(void *argument)
{
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);

    for (;;) {
        osSemaphoreAcquire(receiveDataSemHandle, osWaitForever);

        webCommand = rxByte;
        osSemaphoreRelease(distanceMeasureSemHandle);   // Liberar distanceMeasureSemHandle
    }
}


void measureDistanceTask(void * argument) {
	//Inicialización VL53
	for(;;) {
		osSemaphoreAcquire(distanceMeasureSemHandle, osWaitForever);
		//CODIGO DE IKER DE LECTURA
		//distancia = readVL53()
		osSemaphoreRelease(distanceProcessSemHandle);
	}
}

void processDistanceTask(void * argument) {
	for(;;) {
		osSemaphoreAcquire(distanceProcessSemHandle, osWaitForever);
		/*
		 * if (distance < 50) {
		 * 	 acceptableDistance = false;
		 * } else {
		 * 	 acceptableDistance = true;
		 * }
		 *
		 * */
		osSemaphoreRelease(processDataSemHandle);
	}
}

void processWebDataTask(void * argument) {
	for(;;) {
		osSemaphoreAcquire(processDataSemHandle, osWaitForever);

		uint8_t cmd = webCommand;

		if (measuringSoil || !acceptableDistance) {
			cmd = 100;
		}

		switch(cmd) {
			case 0:
				//NO SE MANDÓ NINGÚN COMANDO
				break;
			case 1:
				//ADELANTE
				break;
			case 2:
				//ÁTRAS
				break;
			case 3:
				//DERECHA
				steering_angles = ChangeRoverDirection(&htim1, steering_angles, RIGHT);
				break;
			case 4:
				//IZQUIERDA
				steering_angles = ChangeRoverDirection(&htim1, steering_angles, LEFT);
				break;
			case 5:
				//SENSOR DE SUELO
				startSoilMeasuting = true;
				break;
			case 100:
				//MEDICIÓN DE SUELO O DISTANCIA NO ACEPTABLE
				break;
			default:
				break;
		}

		if (startSoilMeasuting) {
		    startSoilMeasuting = false;
		    measuringSoil = true;          //TODO: PONER EN FALSE CUANDO TERMINE LA TASK DE MEDIR SUELO
		    osSemaphoreRelease(soilMeasureSemHandle);
		}
	}
}
