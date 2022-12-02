#ifndef USER_CODE_H_
#define USER_CODE_H_
#include "CLCD_I2C.h"

extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart2;

extern PCD_HandleTypeDef hpcd_USB_FS;

void setup();
void loop();

void blinkCursor();
void updateTime();

void cbShort();
void cbLong();
void cbDouble();
#endif
