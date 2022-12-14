#include "BUTTON.h"

// ************************** Low Level Layer *****************************************************//
static ButtonState BUTTON_ReadPin(Button *Button) {
  return HAL_GPIO_ReadPin(Button->Port, Button->Pin);
}
// ************************** High Level Layer *****************************************************//
void buttonInit(Button *button, GPIO_TypeDef *BUTTON_PORT, uint16_t BUTTON_Pin) {
  button->Port = BUTTON_PORT;
  button->Pin = BUTTON_Pin;
}

/*
 * true if short click
 * false if long click
 * */
uint8_t isShortClick(Button *button) {
  if (button->timePress > DEBOUND_TIME && button->timePress <= SINGLE_CLICK_TIME) {
	return 1;
  } else if (button->timePress > SINGLE_CLICK_TIME)
	return 2;
  else
	return 0;
}
