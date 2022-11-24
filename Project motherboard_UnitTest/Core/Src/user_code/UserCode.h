#ifndef _USERCODE_H_
#define _USERCODE_H_

#include "BUTTON.h"
#include "CLCD_I2C.h"

#include "setting.h"
#include "int_def.h"

#include "stdlib.h"

#include "test_handler.h"

#define FLASH_ADDR_PAGE_126 (u32)0x0801F810
#define FLASH_ADDR_PAGE_127 (u32)0x0801FC00
#define FLASH_USER_START_ADDR	FLASH_ADDR_PAGE_126
#define FLASH_USER_END_ADDR     FLASH_ADDR_PAGE_127 + FLASH_PAGE_SIZE

extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern u8 state;
void start_up();
void main_loop();

void LED_ON(LED led);
void LED_OFF(LED led);
void LED_TOGGLE(LED led);

void FLASH_WritePage(uint32_t startPage, uint32_t endPage, uint32_t data);
uint32_t FLASH_ReadData(uint32_t addr);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif
