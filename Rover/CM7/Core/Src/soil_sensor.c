#include "soil_sensor.h"
extern UART_HandleTypeDef SOIL_UART;

#define SOIL_ADRR 0x01
#define SOIL_FUNC_READ 0x03
#define SOIL_NREGS 7
#define SOIL_RESP_LEN (5 + 2 * SOIL_NREGS)

static uint16_t modbus_crc(uint8_t *buf, uint8_t len){
	uint16_t crc = 0xFFFF;

	for (int i = 0; i < len; i++){
		crc ^= buf[i];
		for (int j = 0; j < 8; j++){
			if (crc & 1){
				crc = (crc >> 1) ^ 0xA001;
			} else {
				crc = crc >>1;
			}
		}
	}
	return crc;
}

SoilStatus Soil_Read(SoilData * data){
	uint8_t req[8] = {SOIL_ADRR, SOIL_FUNC_READ, 0x00, 0x00, 0x00, SOIL_NREGS};
	uint8_t resp[SOIL_RESP_LEN] = {0};
	uint16_t crc = modbus_crc(req,6);
	req[6] = crc & 0xFF; //low
	req[7] = crc >> 8; //high

	HAL_UART_Transmit(&SOIL_UART, req, 8, 100);

	if (HAL_UART_Receive(&SOIL_UART, resp, SOIL_RESP_LEN, 1000) != HAL_OK) {
	    return SOIL_TIMEOUT;
	}

	uint16_t crc_r = modbus_crc(resp,SOIL_RESP_LEN-2);
	uint8_t low = crc_r & 0xFF;
	uint8_t high = crc_r >> 8;

	if ((low != resp[SOIL_RESP_LEN-2]) || (high != resp[SOIL_RESP_LEN-1])){
		return SOIL_CRC_ERROR;
	}

	uint16_t valor = (resp[3] << 8) | resp[4];
	data->humedad = valor / 10.0f;
	valor = (resp[5] << 8) | resp[6];
	data->temperatura = (int16_t)valor / 10.0f;
	valor = (resp[7] << 8) | resp[8];
	data->conductividad = valor;
	valor = (resp[9] << 8) | resp[10];
	data->ph = valor / 10.0f;
	valor = (resp[11] << 8) | resp[12];
	data->nitrogeno = valor;
	valor = (resp[13] << 8) | resp[14];
	data->fosforo = valor;
	valor = (resp[15] <<8) | resp[16];
	data->potasio = valor;

	return SOIL_OK;
}







