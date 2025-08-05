#ifndef __CONFIG_H
#define __CONFIG_H

#include "main.h"
#include "stm32c0xx_hal.h"

extern TIM_HandleTypeDef htim1;		// TIM for 3 phase PWM
extern TIM_HandleTypeDef htim14;	// TIM for delay us
extern TIM_HandleTypeDef htim16;	// TIM for detect zero crossing time
extern TIM_HandleTypeDef htim17;	// TIM for delay T/2

#define TIM_PWM_SOURCE		&htim1
#define TIM_PWM_PHASE_A		TIM_CHANNEL_1
#define TIM_PWM_PHASE_B		TIM_CHANNEL_2
#define TIM_PWM_PHASE_C		TIM_CHANNEL_3

#define TIM_DELAY_SOURCE	&htim14

void delay_us(uint16_t time);

#endif