#include "trapezoidal_control.h"

uint8_t bldc_step = 0;

uint16_t last_zc_time = 0;
uint16_t current_zc_time = 0;

uint8_t zc_detect_cnt = 0;
uint8_t startup_mode = 1;

#define ZC_DETECT_THRESHOLD 5  // Number of valid ZC detections to exit startup

/* 
-------------IR2101 DRIVER-------------
GPIO_SET	: MOSFET ON (CONDUCTING)
GPIO_RESET: MOSFET OFF (NOT CONDUCTING)
*/

void step0(void){	// AH_CL
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_RESET); // AL Floating
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_RESET); // BL Floating
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_B); // PWM Phase B off
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_C); // PWM Phase C off
	delay_us(1);
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_SET); // CL GND
	HAL_TIM_PWM_Start(TIM_PWM_SOURCE, TIM_PWM_PHASE_A);		 // AH PWM
}

void step1(void){	//BH_CL
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_RESET); // AL Floating
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_RESET); // BL Floating
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_C); // PWM Phase C off
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_A); // PWM Phase A off
	delay_us(1);
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_SET); // CL GND
	HAL_TIM_PWM_Start(TIM_PWM_SOURCE, TIM_PWM_PHASE_B);		 // BH PWM
}

void step2(void){	//BH_AL
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_RESET); // BL Floating
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_RESET); // CL Floating
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_A); // PWM Phase A off
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_C); // PWM Phase C off
	delay_us(1);
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_SET); // AL GND
	HAL_TIM_PWM_Start(TIM_PWM_SOURCE, TIM_PWM_PHASE_B);		 // BH PWM
}

void step3(void){	//CH_AL
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_RESET); // BL Floating
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_RESET); // CL Floating
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_A); // PWM Phase A off
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_B); // PWM Phase B off
	delay_us(1);
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_SET); // AL GND
	HAL_TIM_PWM_Start(TIM_PWM_SOURCE, TIM_PWM_PHASE_C);		 // CH PWM
}

void step4(void){	//CH_BL
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_RESET); // AL Floating
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_RESET); // CL Floating
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_A); // PWM Phase A off
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_B); // PWM Phase B off
	delay_us(1);
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_SET); // BL GND
	HAL_TIM_PWM_Start(TIM_PWM_SOURCE, TIM_PWM_PHASE_C);		 // CH PWM
}

void step5(void){	//AH_BL
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_RESET); // AL Floating
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_RESET); // CL Floating
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_B); // PWM Phase B off
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_C); // PWM Phase C off
	delay_us(1);
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_SET); // BL GND
	HAL_TIM_PWM_Start(TIM_PWM_SOURCE, TIM_PWM_PHASE_A);		 // AH PWM
}

void off_all_phase(void){
	HAL_GPIO_WritePin(AL_GPIO_Port, AL_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BL_GPIO_Port, BL_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CL_GPIO_Port, CL_Pin, GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_A);
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_B);
	HAL_TIM_PWM_Stop(TIM_PWM_SOURCE, TIM_PWM_PHASE_C);
}

void bldc_move(void){
	switch(bldc_step){
		case 0: step0(); break;
		case 1: step1(); break;
		case 2: step2(); break;
		case 3: step3(); break;
		case 4: step4(); break;
		case 5: step5(); break;
		default: off_all_phase(); break;
	}
}

void bldc_startup(void){
	uint16_t startup_delay = 10000;
	zc_detect_cnt = 0;
	
	while(startup_mode){
		bldc_move();
		bldc_step = (bldc_step + 1) % 6;
		delay_us(startup_delay);
		if(startup_delay > 3000){
			startup_delay -= 300;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_8 || GPIO_Pin == GPIO_PIN_9 || GPIO_Pin == GPIO_PIN_10){
		current_zc_time = __HAL_TIM_GET_COUNTER(&htim16);
		uint16_t T2_delay = (current_zc_time >= last_zc_time) ? 
												(current_zc_time - last_zc_time) :
												(0xFF - last_zc_time + current_zc_time);
		
		__HAL_TIM_SET_COUNTER(&htim17, 0);
		__HAL_TIM_SET_AUTORELOAD(&htim17, (T2_delay >> 1));
		HAL_TIM_Base_Start_IT(&htim17);
		
		last_zc_time = current_zc_time;
		
		if(startup_mode){
			zc_detect_cnt++;
			if(zc_detect_cnt >= ZC_DETECT_THRESHOLD){
				startup_mode = 0; // Exit startup mode after enough ZC detections
			}
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM17){
		HAL_TIM_Base_Stop_IT(&htim17);
		bldc_step = (bldc_step + 1) % 6;
		bldc_move();
	}
}
