#ifndef __BUTTON_H
#define __BUTTON_H

#include "main.h"
#include "stdbool.h"
#include "stm32f1xx_hal.h"
#include "stdint.h"

#define DEBOUNCE_TIME 50
#define CLICK_TIME 200
#define PRESS_TIME 400
#define MAXCLICKS 2

typedef enum {
	BTN_RESET = 0, BTN_SET = 1
} ButtonState;

typedef enum stateMachine_t {
	INIT = 0, DOWN = 1, UP = 2, COUNT = 3, PRESS = 6, PRESSEND = 7, UNKNOWN = 99
} StateMachine;

typedef void (*CallbackShortPress)();
typedef void (*CallbackLongPress)();
typedef void (*CallbackDoublePress)();

typedef struct {
	GPIO_TypeDef *Port;
	uint16_t Pin;

	uint32_t timeIn;
	uint32_t timeDelta;
	bool hasChanged;
	bool _change;

	uint32_t timePress;
	uint32_t startPress;
	uint32_t stopPress;

	StateMachine lastState;
	StateMachine state;

	int nClicks;

	CallbackShortPress callbackShort;
	CallbackLongPress callbackLong;
	CallbackDoublePress callbackDouble;
} Button;

void buttonInit(Button *button, GPIO_TypeDef *BUTTON_PORT, uint16_t BUTTON_Pin,
		CallbackShortPress cbShort, CallbackLongPress cbLong,
		CallbackDoublePress cbDouble);
void _newState(Button *button, StateMachine nextState);
void _reset(Button *button);
void tick(Button *button, GPIO_PinState btnState);

#endif

