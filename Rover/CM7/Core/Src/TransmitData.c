#include "TransmitData.h"

#define TX_UART huart1
#define TX_BUFFER_LEN 128

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef TX_UART;


float temperatura;
float humedad;
float presion;

float indiceUV;
UV_Riesgo_t riesgoUV;

extern volatile bool vl53Failed;
extern bool soilSensorFailed;

bool bmeFailed = true;
bool uvFailed  = true;

extern SoilData soilData;

void readEnvironmentSensorsTask(void *argument)
{
    uint8_t bmeOK = 0;
    BME280_Datos bmeData;

    // Inicialización del BME280
    osMutexAcquire(i2c2MutexHandle, osWaitForever);

    if (BME280_Init(&hi2c2) == 1) {
        bmeOK = BME280_Calibrar();
    }

    osMutexRelease(i2c2MutexHandle);

    UV_Status_t uvStatus = UV_new();

    for (;;)
    {
    	osMutexAcquire(i2c2MutexHandle, osWaitForever);

    	if (bmeOK && BME280_Leer(&bmeData)) {
    		temperatura = bmeData.temperatura / 100.0f;
    		humedad     = bmeData.humedad     / 100.0f;
    		presion     = bmeData.presion     / 100.0f;
    		bmeFailed = false;
    	} else {
    		bmeFailed = true;
    		bmeOK = BME280_Init(&hi2c2) && BME280_Calibrar();
    	}

    	osMutexRelease(i2c2MutexHandle);

    	uvStatus = UV_new();
    	uvFailed = (uvStatus != UV_OK);
    	if (!uvFailed) {
    		indiceUV = uv.index;
    		riesgoUV = uv.riesgo;
    	}

        osSemaphoreRelease(guvaProcessSemHandle);

        osDelay(2500);
    }
}

void processGUVAsensorTask(void *argument) {
	for(;;) {
		osSemaphoreAcquire(guvaProcessSemHandle, osWaitForever);

		//Lo que diga el profe Oscar :)

		osSemaphoreRelease(transmitDataSemHandle);
	}
}

void TransmitDataTask(void *argument) {
	char buffer[TX_BUFFER_LEN];

	for (;;) {
		osSemaphoreAcquire(transmitDataSemHandle, osWaitForever);

		SoilData s;
		osKernelLock();
		s = soilData;
		osKernelUnlock();

		uint8_t estado = (bmeFailed << 0) |
		                 (uvFailed << 1) |
		                 (soilSensorFailed << 2) |
		                 (vl53Failed << 3);

		// Orden: temp, hum, presion(hPa), indiceUV, riesgoUV,
		//        humSuelo, tempSuelo, conductividad, ph, N, P, K, estado
		int len = snprintf(buffer, sizeof(buffer),
				"%.2f,%.2f,%.2f,%.2f,%d,%.1f,%.1f,%u,%.1f,%u,%u,%u,%u\n",
				temperatura, humedad, presion, indiceUV, (int)riesgoUV,
				s.humedad, s.temperatura, (unsigned)s.conductividad, s.ph,
				(unsigned)s.nitrogeno, (unsigned)s.fosforo, (unsigned)s.potasio,
				(unsigned)estado);

		if (len > 0 && len < (int)sizeof(buffer)) {
			HAL_UART_Transmit(&TX_UART, (uint8_t *)buffer, (uint16_t)len, 200);
		}
	}
}
