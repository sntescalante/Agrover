#include "RTOS_Rover.h"

osThreadId_t measureEnvironmentSensorsHandle = {
		.name = "MeasureEnvironmentSensors",
		.stack_size = 256*4,
		.priority = (osPriority_t) osPriorityLow
};

