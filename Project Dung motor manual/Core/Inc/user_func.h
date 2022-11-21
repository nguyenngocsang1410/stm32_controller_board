/*
 * user_func.h
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

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;


uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax);

void set_motor(u8 id, s8 dir, u16 val);
void startup();
void lightLED(bool yellow, bool green);
void returnHome();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
#endif /* INC_USER_FUNC_H_ */
