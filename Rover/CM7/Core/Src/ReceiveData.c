#include "ReceiveData.h"

extern UART_HandleTypeDef huart1;

extern ReceiveWifiDataConfig configWifi;

uint8_t rxByte;
uint8_t webCommand;

//Variables VL53 e I2C
#define VL53_TIMEOUT_MEDICION_MS  100
#define VL53_TIMEOUT_CALIB_MS     500
#define VL53_REINTENTO_MS         1000
#define DISTANCIA_MINIMA_MM       150

uint16_t distance_mm = 0;
volatile bool vl53Failed = true;
volatile bool distanciaValida = false;

extern I2C_HandleTypeDef hi2c2;

//Variable Sensor suelo

#define SOIL_INTENTOS  3

extern SoilData soilData;
bool newSoilValues = false;
bool soilSensorFailed = false;

//FLAGS DE SUELO Y DISTANCIA GLOBALES
extern volatile bool measuringSoil;
extern volatile bool acceptableDistance;

//Flag para iniciar medición de suelo
bool volatile startSoilMeasuting = false;

//Timers PWM Motores
extern TIM_HandleTypeDef htim1;

//Angulos servos
extern SteeringAngles steering_angles;



#define PERIODO_DISTANCIA_MS  100

volatile bool comandoNuevo = false;
extern uint8_t current_direction;


void receiveDataTask(void *argument)
{
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);

    for (;;) {
    		osSemaphoreAcquire(receiveDataSemHandle, osWaitForever);

    		webCommand = rxByte;
    		comandoNuevo = true;
    		osSemaphoreRelease(distanceMeasureSemHandle);
    	}
}

static bool VL53_Arrancar(void)
{
    if (!VL53_Init(&hi2c2)) {
        return false;
    }

    uint8_t estado;
    uint32_t t0 = osKernelGetTickCount();
    do {
        estado = VL53_Calibrar();
        if (estado != VL53_OCUPADO) break;
        osDelay(1);
    } while ((osKernelGetTickCount() - t0) < VL53_TIMEOUT_CALIB_MS);

    return estado == VL53_OK;
}

// Una medición completa con tiempo límite.
static bool VL53_Medir(uint16_t *mm)
{
    if (!VL53_IniciarMedicion()) {
        return false;            // el sensor perdió su configuración o no responde
    }

    uint32_t t0 = osKernelGetTickCount();
    while (!VL53_DatoListo()) {
        if ((osKernelGetTickCount() - t0) >= VL53_TIMEOUT_MEDICION_MS) {
            return false;
        }
        osDelay(1);
    }

    return VL53_LeerDistancia(mm);
}


void measureDistanceTask(void *argument)
{
    osMutexAcquire(i2c2MutexHandle, osWaitForever);
    vl53Failed = !VL53_Arrancar();
    osMutexRelease(i2c2MutexHandle);
    uint32_t ultimoIntento = osKernelGetTickCount();

    for (;;) {
    	osSemaphoreAcquire(distanceMeasureSemHandle, PERIODO_DISTANCIA_MS);

        bool ok = false;
        uint16_t lectura = 0;

        osMutexAcquire(i2c2MutexHandle, osWaitForever);

        // Si el sensor está caído, reintentar arrancarlo (máximo 1 vez por segundo)
        if (vl53Failed && (osKernelGetTickCount() - ultimoIntento) >= VL53_REINTENTO_MS) {
            vl53Failed = !VL53_Arrancar();
            ultimoIntento = osKernelGetTickCount();
        }

        if (!vl53Failed) {
            ok = VL53_Medir(&lectura);
            if (!ok) {
                vl53Failed = true;
            }
        }

        osMutexRelease(i2c2MutexHandle);

        if (ok) {
            distance_mm = lectura;
        }
        distanciaValida = ok;

        osSemaphoreRelease(distanceProcessSemHandle);
    }
}

void processDistanceTask(void *argument)
{
    for (;;) {
        osSemaphoreAcquire(distanceProcessSemHandle, osWaitForever);

        acceptableDistance = distanciaValida && (distance_mm >= DISTANCIA_MINIMA_MM);

        osSemaphoreRelease(processDataSemHandle);
    }
}

void processWebDataTask(void * argument) {
	for(;;) {
		osSemaphoreAcquire(processDataSemHandle, osWaitForever);

				osKernelLock();
				bool esComando = comandoNuevo;
				uint8_t cmd = webCommand;
				comandoNuevo = false;
				osKernelUnlock();

				if (!esComando) {
					if (!acceptableDistance && current_direction == FORWARD) {
						Stop();
					}
					continue;
				}

				if (measuringSoil || (!acceptableDistance && cmd == 1)) {
					cmd = 100;
				}

		switch(cmd) {
			case 0:
				//NO SE MANDÓ NINGÚN COMANDO
				Stop();
				break;
			case 1:
				//ADELANTE
				MoveForward();
				break;
			case 2:
				//ÁTRAS
				MoveBackwards();
				break;
			case 3:
				//DERECHA
				Stop();
				steering_angles = ChangeRoverDirection(&htim1, steering_angles, RIGHT);
				break;
			case 4:
				Stop();
				//IZQUIERDA
				steering_angles = ChangeRoverDirection(&htim1, steering_angles, LEFT);
				break;
			case 5:
				Stop();
				//SENSOR DE SUELO
				startSoilMeasuting = true;
				break;
			case 100:
				//MEDICIÓN DE SUELO O DISTANCIA NO ACEPTABLE
				Stop();
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

void soilMeasuringTask(void * argument) {
	for (;;) {
		osSemaphoreAcquire(soilMeasureSemHandle, osWaitForever);

		//TODO: AGREGAR MOVIMIENTO DE MOTORES PARA CLAVAR SENSOR

		SoilData lectura;
		SoilStatus st = SOIL_TIMEOUT;

		for (uint8_t i = 0; i < SOIL_INTENTOS; i++) {
			st = Soil_Read(&lectura);
			if (st == SOIL_OK) {
				break;
			}
			osDelay(200);
		}

		if (st == SOIL_OK) {
			osKernelLock();
			soilData = lectura;
			osKernelUnlock();
			soilSensorFailed = false;
			newSoilValues = true;
			osSemaphoreRelease(soilDataProcessSemHandle);
		} else {
			soilSensorFailed = true;
		}

		measuringSoil = false;
	}
}

void soilDataProcessTask(void * argument) {
	for(;;) {
		osSemaphoreAcquire(soilDataProcessSemHandle, osWaitForever);

		if (soilData.humedad < 30.0f || (soilData.temperatura > 30.0f && soilData.humedad < 40.0f)) {
			osSemaphoreRelease(wateringSemHandle);
		}

	}
}

void wateringTask(void * argument) {
	for(;;) {
		osSemaphoreAcquire(wateringSemHandle, osWaitForever);
		HAL_GPIO_WritePin(GPIOD, Bomba_Pin, GPIO_PIN_SET);
		osDelay(1000);
		HAL_GPIO_WritePin(GPIOD, Bomba_Pin, GPIO_PIN_RESET);
	}
}
