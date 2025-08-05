#include "config.h"

void delay_us(uint16_t time){
	htim14.Instance->CNT = 0;
	HAL_TIM_Base_Start(&htim14);
	while(htim14.Instance->CNT < time);
	HAL_TIM_Base_Stop(&htim14);
}
