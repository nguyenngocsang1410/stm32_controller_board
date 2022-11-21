/*
 * user_func.h (FINAL x3)
 *
 *  Created on: Sep 20, 2022
 *      Author: SangNguyen
 */

#ifndef INC_USER_FUNC_H_
#define INC_USER_FUNC_H_

#include "main.h"
#include "BUTTON.h"

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;

typedef enum {
  IDLE = 1, RUN = 0
} State_t;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart2;

extern Button BTN_Start, BTN_Stop, SW_Mode, SW_Lvl1, SW_Lvl2;
extern bool END_REACH, BEGIN_REACH, MID_REACH;

extern State_t state;

uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax);

void set_motor_velo(u8 id, s16 val);
void set_motor(u8 id, s8 dir, u16 val);
void stop_motor(bool motor1, bool motor2);

void lightLED(bool yellow, bool green);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void start_up();

void active_idle();
void active_run();
void check_button();
void flash_LED(u8 id, u16 interval);
void return_home();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void apply_state();

#endif /* INC_USER_FUNC_H_ */
