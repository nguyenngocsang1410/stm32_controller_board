#include "BUTTON.h"
// (FINAL x3)

// ************************** Low Level Layer *****************************************************//
static ButtonState BUTTON_ReadPin(Button *Button) {
	return HAL_GPIO_ReadPin(Button->Port, Button->Pin);
}
// ************************** High Level Layer *****************************************************//
void buttonInit(Button *button, GPIO_TypeDef *BUTTON_PORT, uint16_t BUTTON_Pin,
		CallbackShortPress cbShort, CallbackLongPress cbLong,
		CallbackDoublePress cbDouble) {
	button->Port = BUTTON_PORT;
	button->Pin = BUTTON_Pin;

	button->state = BUTTON_ReadPin(button);
	button->lastState = button->state;
	button->timeIn = HAL_GetTick();
	button->hasChanged = false;
	button->_change = false;
	button->nClicks = 0;

	button->callbackShort = cbShort;
	button->callbackLong = cbLong;
	button->callbackDouble = cbDouble;
}

//Source

void _newState(Button *button, StateMachine nextState) {
	button->lastState = button->state;
	button->state = nextState;
}

void _reset(Button *button) {
	button->state = INIT;
	button->lastState = INIT;
	button->nClicks = 0;
	button->startPress = 0;
}

#define BUTTON_DOWN _btnState == GPIO_PIN_RESET
#define BUTTON_UP _btnState == GPIO_PIN_SET

void tick(Button *button, GPIO_PinState _btnState) {

	uint32_t now = HAL_GetTick();

	button->timePress = now - button->startPress;

	switch (button->state) {
	case INIT:
		if (BUTTON_DOWN) {
			_newState(button, DOWN);
			button->startPress = now;
			button->nClicks = 0;
		}
		break;
	case DOWN:
		if ((BUTTON_UP) && button->timePress < DEBOUNCE_TIME) {
			_newState(button, button->lastState);
		} else if (BUTTON_UP) {
			_newState(button, UP);
			button->startPress = now;
		} else if ((BUTTON_DOWN) && button->timePress > PRESS_TIME) {
			_newState(button, PRESS);
		}
		break;
	case UP:
		if ((BUTTON_UP) && button->timePress < DEBOUNCE_TIME) {
			_newState(button, button->lastState);
		} else if (button->timePress >= DEBOUNCE_TIME) {
			button->nClicks++;
			_newState(button, COUNT);
		}
		break;
	case COUNT:
		if (BUTTON_DOWN) {
			_newState(button, DOWN);
			button->startPress = now;
		} else if ((button->timePress > CLICK_TIME)
				|| (button->nClicks == MAXCLICKS)) {
			if (button->nClicks == 1) {
				// Single Click Function here
				button->callbackShort();
			} else if (button->nClicks == 2) {
				// Double Click Function here
				button->callbackDouble();
			}
			_reset(button);
		}
		break;
	case PRESS:
		if (BUTTON_UP) {
			_newState(button, PRESSEND);
			button->startPress = now;
		} else {
		}
		break;
	case PRESSEND:
		if ((BUTTON_DOWN) && button->timePress < DEBOUNCE_TIME) {
			_newState(button, button->lastState);
		} else if (button->timePress >= DEBOUNCE_TIME) {
			//PRESS Function here
			button->callbackLong();
			_reset(button);
		}
		break;
	default:
		_newState(button, INIT);
		break;
	}

}
