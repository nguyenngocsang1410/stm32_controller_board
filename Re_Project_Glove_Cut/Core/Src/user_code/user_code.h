#ifndef _USERCODE_H_
#define _USERCODE_H_

#include "setting.h"
#include "main_loop.h"
#include "int_def.h"
#include "BUTTON.h"
#include "CLCD_I2C.h"
#include "it_func.h"
#include "pid_code.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define FLASH_ADDR_PAGE_126 (u32)0x0801F810
#define FLASH_ADDR_PAGE_127 (u32)0x0801FC00
#define FLASH_USER_START_ADDR    FLASH_ADDR_PAGE_126
#define FLASH_USER_END_ADDR     (FLASH_ADDR_PAGE_127 + FLASH_PAGE_SIZE)

#define TIM1_count TIM1->CNT
#define TIM2_count TIM2->CNT

extern u8 mSpeed;
extern u32 mSec;
extern s32 mNumLoopCount;
extern u16 nLoop;
extern u16 lastLoop;

extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern u8 state;
extern CLCD_I2C_Name LCD;
extern Button BTN_Start, BTN_Stop, BTN_Mode;
extern bool firstMinSelected, lastMinSelected;
extern bool firstSecSelected, lastSecSelected;

extern float curSpeed, preSpeed;
extern u32 curPos, prePos, pre_posi;
extern u32 curTime, preTime;

extern u32 pwm;
extern const u32 timeInterval; //ms

extern bool menu1_selected; //enable/disable to change the value of menu item
extern bool menu2_selected;
extern bool menu3_selected;
extern bool menu4_selected;

extern bool refreshMenu1;
extern bool refreshMenu2;
extern bool refreshMenu3;

extern bool refreshSelection; //refreshes selection (> / X)
extern bool refreshLCD; //refreshes values
extern bool refreshClock;

extern u8 menuCounter; //counts the clicks of the rotary encoder between menu items (0-3 in this case)
extern u8 menu2_subCounter;

extern u8 firstMin, lastMin, firstSec, lastSec;
extern u32 nSec;
extern u32 PROBE_ON, PROBE_OFF;

extern bool FLAG_run;

extern float v_ref;
extern float kp, ki, kd;

extern PidObject PID_motor;

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

s32 wsub(u32 a, u32 b, u32 threshold);

s32 wsub2(u32 a, u32 b);

#endif
