#include "UserCode.h"
#include "stdio.h"
#include "BUTTON.h"
#include "main.h"

#define x1_pin X1_GPIO_Port, X1_Pin
#define x7_pin X7_GPIO_Port, X7_Pin

Button x1;
Button x7;
CLCD_I2C_Name LCD;
GPIO_PinState a;
GPIO_PinState b;

u32 mSec = 0;
u8 timeHolder[4] = { 0 };
u8 holderIndex = 0; // min1 min2 : min3 min4
bool menuSelected = false;
bool digitSelected = false;

bool _isCursorBlink = false;
u32 lastBlink = 0;
void setup() {
//  HAL_TIM_Base_Start_IT(&htim4);

  LCD_Begin(&LCD, &hi2c2, 0x4E, 16, 2);
  LCD_Clear(&LCD);
  LCD_Print_String_At(&LCD, 1, 2, "Begin");

  buttonInit(&x1, x1_pin, cbShort, cbLong, cbDouble);
//  buttonInit(&x7, x7_pin, cbShort, cbLong, cbDouble);

  LCD_Print_String_At(&LCD, 1, 1, "TIME: ");
  LCD_Print_String_At(&LCD, 2, 1, "SET: ");
  updateTime();
}
void loop() {
  a = HAL_GPIO_ReadPin(x1_pin);
  tick(&x1, a);

  if (menuSelected && !digitSelected) {
	u32 now = HAL_GetTick();
	if (now - lastBlink > 300) {
	  lastBlink = now;
	  if (_isCursorBlink) {
		_isCursorBlink = false;
		CLCD_I2C_CursorOn(&LCD);
	  } else {
		_isCursorBlink = true;
		CLCD_I2C_CursorOff(&LCD);
	  }
	}
  }
}

void updateTime() {
  char holder[3];
  for (u8 i = 0; i < 4; i++) {
	switch (i) {
	  case 0:
		LCD_Set_Cursor(&LCD, 1, 7);
		sprintf(holder, "%2d", timeHolder[i]);
		break;
	  case 1:
		LCD_Set_Cursor(&LCD, 1, 9);
		sprintf(holder, "%1d", timeHolder[i]);
		break;
	  case 2:
		LCD_Set_Cursor(&LCD, 1, 11);
		sprintf(holder, "%1d", timeHolder[i]);
		break;
	  case 3:
		LCD_Set_Cursor(&LCD, 1, 12);
		sprintf(holder, "%1d", timeHolder[i]);
		break;
	  default:
		break;
	}
	LCD_Print_String(&LCD, holder);
  }
  LCD_Print_String_At(&LCD, 1, 10, ":");
}

/*
 * click -> inc
 * double click -> choose
 * hold -> back
 * */

void cbShort() {
  LCD_Print_String_At(&LCD, 2, 11, "SHORT");

  if (menuSelected) {
	if (digitSelected) {
	  timeHolder[holderIndex]++;
	} else {
	  holderIndex++;
	  if (holderIndex > 3)
		holderIndex = 0;
	}
  }

  updateTime();
  switch (holderIndex) {
	case 0:
	  LCD_Set_Cursor(&LCD, 1, 8);
	  break;
	case 1:
	  LCD_Set_Cursor(&LCD, 1, 9);
	  break;
	case 2:
	  LCD_Set_Cursor(&LCD, 1, 11);
	  break;
	case 3:
	  LCD_Set_Cursor(&LCD, 1, 12);
	  break;
	default:
	  break;
  }
}

void cbLong() {
  LCD_Print_String_At(&LCD, 2, 11, "LONG ");
  if (menuSelected == true) {
	if (digitSelected == true) {
	  digitSelected = false;
	} else {
	  menuSelected = false;
	  CLCD_I2C_BlinkOff(&LCD);
	}
  }
  switch (holderIndex) {
	case 0:
	  LCD_Set_Cursor(&LCD, 1, 8);
	  break;
	case 1:
	  LCD_Set_Cursor(&LCD, 1, 9);
	  break;
	case 2:
	  LCD_Set_Cursor(&LCD, 1, 11);
	  break;
	case 3:
	  LCD_Set_Cursor(&LCD, 1, 12);
	  break;
	default:
	  break;
  }
}
void cbDouble() {
  LCD_Print_String_At(&LCD, 2, 11, "DOUBL");
  if (menuSelected == true) {
	if (digitSelected == false) {
	  CLCD_I2C_CursorOff(&LCD);
	  CLCD_I2C_BlinkOn(&LCD);
	  digitSelected = true;
	}
  } else {
	holderIndex = 0;
	menuSelected = true;
	digitSelected = false;
  }
  switch (holderIndex) {
	case 0:
	  LCD_Set_Cursor(&LCD, 1, 8);
	  break;
	case 1:
	  LCD_Set_Cursor(&LCD, 1, 9);
	  break;
	case 2:
	  LCD_Set_Cursor(&LCD, 1, 11);
	  break;
	case 3:
	  LCD_Set_Cursor(&LCD, 1, 12);
	  break;
	default:
	  break;
  }
}
