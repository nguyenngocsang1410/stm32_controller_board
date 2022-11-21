#include "BUTTON.h"
// (FINAL x3)

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
  Button->_change = false;
}

ButtonState buttonRead(Button *Button) {
  ButtonState curState = BUTTON_ReadPin(Button);
  uint32_t now = HAL_GetTick();
  if (curState != Button->state) {
	if (Button->_change == false)
	{
	  Button->timeIn = now;
	  Button->_change = true;
	}
	Button->timeDelta = now - Button->timeIn;
	if (Button->timeDelta > DEBOUND_TIME) {
	  Button->state = curState;
	  Button->timeDelta = 0;
	  Button->timeIn = now;
	  Button->_change = false;
	  Button->hasChanged = true;
	}
  } else if (curState == Button->state)
  {
	Button->_change = false;
  }
  return Button->state;
}

