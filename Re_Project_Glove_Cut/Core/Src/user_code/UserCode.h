#ifndef _USERCODE_H_
#define _USERCODE_H_

#include "setting.h"
#include "int_def.h"
#include "BUTTON.h"
#include "CLCD_I2C.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define FLASH_ADDR_PAGE_126 (u32)0x0801F810
#define FLASH_ADDR_PAGE_127 (u32)0x0801FC00
#define FLASH_USER_START_ADDR    FLASH_ADDR_PAGE_126
#define FLASH_USER_END_ADDR     (FLASH_ADDR_PAGE_127 + FLASH_PAGE_SIZE)

extern u8 mSpeed;
extern u32 mSec;
extern s32 mNumLoopCount;
extern u16 nLoop;

#define TIM1_count TIM1->CNT
#define TIM2_count TIM2->CNT

extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern u8 state;

void setup();

void loop();

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void check_state();

void check_ecd();

void LED_ON();

void LED_OFF();

void LED_TOGGLE();

void FLASH_WritePage(uint32_t startPage, uint32_t endPage, uint32_t data);

uint32_t FLASH_ReadData(uint32_t addr);

void printDefaultLCD();

void update_Menu1();

void update_Menu2();

void update_Menu3();

void LCD_Print_Clock(u32 sec);

void printMenuName();

void printMenuValue();

void blinkCursor();

void updateSelection();

void set_motor(u8 id, s8 dir, u16 val);

s32 map(s32 x, s32 in_min, s32 in_max, s32 out_min, s32 out_max);

void reset_state();

void save_menu_value();

void stopCmd();

#endif
