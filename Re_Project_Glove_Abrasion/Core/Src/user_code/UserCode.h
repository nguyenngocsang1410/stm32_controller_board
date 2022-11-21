#ifndef _USERCODE_H_
#define _USERCODE_H_

#include "BUTTON.h"
#include "CLCD_I2C.h"
#include "setting.h"

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;


#define BTN_Start_Pin X0_Pin
#define BTN_Start_GPIO_Port X0_GPIO_Port
#define BTN_Stop_Pin X2_Pin
#define BTN_Stop_GPIO_Port X1_GPIO_Port
#define BTN_Mode_Pin X1_Pin
#define BTN_Mode_GPIO_Port X1_GPIO_Port

#define LED_Panel_Pin Q0_Pin
#define LED_Panel_GPIO_Port Q0_GPIO_Port

#define FLASH_ADDR_PAGE_126 (u32)0x0801F810
#define FLASH_ADDR_PAGE_127 (u32)0x0801FC00
#define FLASH_USER_START_ADDR	FLASH_ADDR_PAGE_126
#define FLASH_USER_END_ADDR     FLASH_ADDR_PAGE_127 + FLASH_PAGE_SIZE

extern ADC_HandleTypeDef hadc1;

extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern u8 state;
void start_up();
void main_loop();

void LCD_Begin(CLCD_I2C_Name *LCD, I2C_HandleTypeDef *hi2c_CLCD,
	uint8_t Address, uint8_t Colums, uint8_t Rows);
void LCD_Set_Cursor(CLCD_I2C_Name *LCD, u8 a, u8 b);
void LCD_Print_Char(CLCD_I2C_Name *LCD, char data);
void LCD_Clear(CLCD_I2C_Name *LCD);
void LCD_Print_String(CLCD_I2C_Name *LCD, char *data);
void LCD_Print_Int(CLCD_I2C_Name *LCD, s32 data);
void LCD_Print_String_At(CLCD_I2C_Name *LCD, u8 a, u8 b, char *data);
void LCD_Print_Int_At(CLCD_I2C_Name *LCD, u8 a, u8 b, s32 data);
void LCD_Return_Home(CLCD_I2C_Name *LCD);

void LED_ON();
void LED_OFF();
void LED_TOGGLE();

void FLASH_WritePage(uint32_t startPage, uint32_t endPage, uint32_t data);
uint32_t FLASH_ReadData(uint32_t addr);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif
