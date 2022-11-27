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

void setup() {
	LCD_Begin(&LCD, &hi2c2, 0x4E, 16, 2);
	LCD_Clear(&LCD);
	LCD_Print_String_At(&LCD, 1, 2, "Begin");

	buttonInit(&x1, x1_pin, cbShort, cbLong, cbDouble);
	buttonInit(&x7, x7_pin, cbShort, cbLong, cbDouble);

}
void loop() {
	a = HAL_GPIO_ReadPin(x1_pin);
	tick(&x1, a);
	b = HAL_GPIO_ReadPin(x7_pin);

	tick(&x7, b);
	HAL_Delay(1);
}

void cbShort() {
	LCD_Clear(&LCD);
	LCD_Print_String_At(&LCD, 1, 1, "SHORT");
}

void cbLong() {
	LCD_Clear(&LCD);
	LCD_Print_String_At(&LCD, 1, 1, "LONG");
}
void cbDouble() {
	LCD_Clear(&LCD);
	LCD_Print_String_At(&LCD, 1, 1, "PRESS");
}
