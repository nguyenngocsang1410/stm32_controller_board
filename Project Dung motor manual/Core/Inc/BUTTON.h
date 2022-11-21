/*****************************************************************************************************
 @File:		Button
 @Author:  khuenguyen
 @website: khuenguyencreator.com
 @youtube: https://www.youtube.com/channel/UCt8cFnPOaHrQXWmVkk-lfvg
 @huong dan su dung:
 - Cau hinh cac chan vao Button la Input_pullup
 - Khoi tao Bien luu gia tri Button:
 BUTTON_Name Button1;
 - Truyen vao chan ten va chan button do
 BUTTON_Init(&Button1, Button_GPIO_Port, Button_Pin);
 - Doc gia tri Button
 Status = Read_Button(&Button1);
 *********************** *****************************************************************************/
#ifndef __BUTTON_H
#define __BUTTON_H

#include "main.h"
#include "stdbool.h"
#include "stm32f1xx_hal.h"

#define DEBOUND_TIME 50

typedef enum {
  BTN_RESET = 0, BTN_SET = 1
} ButtonState;

typedef struct {
  GPIO_TypeDef *Port;
  uint16_t Pin;

  ButtonState preState;
  ButtonState state;
  uint32_t timeIn;
  uint32_t timeDelta;
  bool hasChanged;
} Button;

void buttonInit(Button *Button, GPIO_TypeDef *BUTTON_PORT, uint16_t BUTTON_Pin);
ButtonState buttonRead(Button *Button);

#endif

