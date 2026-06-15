/*
 * led_controller.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Whath
 */

#include "stdint.h"
#include "stm32l4xx_it.h"
#include "stm32l4xx_hal.h"
#include "stdio.h"

void led_controller_pwm(int8_t red, int8_t green, int8_t blue)
{
	//TODO: Change the 3 pwm for controlling the LEDs.

//	HAL_LPTIM_PWM_Start();
//	HAL_LPTIM_PWM_Start();
//	HAL_LPTIM_PWM_Start();
	printf("RGB: %i %i %i", red, green, blue);
}
//void led_controller_pwm(TIM_HandleTypeDef hlptim_red, TIM_HandleTyoeDef hlptim_green, TIM_HandleTyoeDef hlptim_blue, int8_t red, int8_t green, int8_t blue)
