#include "sensor_ultravioleta.h"

extern ADC_HandleTypeDef UV_HADC;

UV_Resultados_t uv = {0};

static UV_Riesgo_t UV_Clasificar_Riesgo(float uv_index){
	if (uv_index < 0) {
		uv_index = 0;
	}

	uint32_t uvi = (uint32_t)(uv_index + 0.5f);

	if (uvi < UV_UMBRAL_BAJO) return UV_RIESGO_BAJO;
	if (uvi < UV_UMBRAL_MODERADO) return UV_RIESGO_MODERADO;
	if (uvi < UV_UMBRAL_ALTO) return UV_RIESGO_ALTO;
	if (uvi < UV_UMBRAL_MUY_ALTO) return UV_RIESGO_MUY_ALTO;
	return UV_RIESGO_EXTREMO;
}

static UV_Status_t UV_Init(void){
	if (HAL_ADCEx_Calibration_Start(&UV_HADC, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK){
		return UV_ERROR_CALIB;
	}
	uv.init = true;
	return UV_OK;
}

static UV_Status_t UV_Leer(void){
	uint32_t sum = 0;

	for (uint32_t i = 0; i < UV_NMUESTRAS; i++){
		if (HAL_ADC_Start(&UV_HADC) != HAL_OK) return UV_ERROR_ADC;
		if (HAL_ADC_PollForConversion(&UV_HADC, UV_ADC_TIMEOUT) != HAL_OK){
			HAL_ADC_Stop(&UV_HADC);
			return UV_ERROR_ADC;
		}
		sum += HAL_ADC_GetValue(&UV_HADC);
		HAL_ADC_Stop(&UV_HADC);
	}

	uv.adc_prom = sum/UV_NMUESTRAS;
	uv.voltaje = (uv.adc_prom * UV_VREF) / UV_ADC_MAX;
	float v_util = uv.voltaje - UV_OFFSET_V;
	if (v_util < 0){
		v_util = 0;
	}

	uv.index = v_util / UV_V_PINDEX;
	uv.riesgo = UV_Clasificar_Riesgo(uv.index);

	return UV_OK;
}

UV_Status_t UV_new(void){
	if (uv.init == 0){
		uv.estado = UV_Init();
		if (uv.estado != UV_OK) return uv.estado;
	}
	uv.estado = UV_Leer();
	return uv.estado;
}
