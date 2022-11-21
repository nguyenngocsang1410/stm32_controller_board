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
  bool _change;
} Button;

void buttonInit(Button *Button, GPIO_TypeDef *BUTTON_PORT, uint16_t BUTTON_Pin);
ButtonState buttonRead(Button *Button);

#endif

