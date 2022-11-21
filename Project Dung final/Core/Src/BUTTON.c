#include "BUTTON.h"

// ************************** Low Level Layer *****************************************************//
static ButtonState BUTTON_ReadPin(Button *Button) {
  return HAL_GPIO_ReadPin(Button->Port, Button->Pin);
}
// ************************** High Level Layer *****************************************************//
void buttonInit(Button *Button, GPIO_TypeDef *BUTTON_PORT, uint16_t BUTTON_Pin) {
  Button->Port = BUTTON_PORT;
  Button->Pin = BUTTON_Pin;

  Button->state = BUTTON_ReadPin(Button);
  Button->preState = Button->state;
  Button->timeIn = HAL_GetTick();
  Button->hasChanged = false;
}

ButtonState buttonRead(Button *Button) {
  ButtonState curState = BUTTON_ReadPin(Button);
  uint32_t now = HAL_GetTick();
  if (curState != Button->state) {
	Button->timeDelta = now - Button->timeIn;
	if (Button->timeDelta > DEBOUND_TIME) {
	  Button->state = curState;
	  Button->timeDelta = 0;
	  Button->timeIn = now;
	  Button->hasChanged = true;
	}
  } else {
	Button->timeIn = now;
  }
  return Button->state;
}

