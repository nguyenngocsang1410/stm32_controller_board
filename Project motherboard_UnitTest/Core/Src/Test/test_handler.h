#ifndef _TEST_HANDLER_H_
#define _TEST_HANDLER_H_

#include "main.h"
#include "../user_code/BUTTON.h"

typedef enum {
  MOTOR_TEST, ENCODER_TEST, INPUT_TEST, OUTPUT_TEST, LCD_TEST
} Test_Type;

typedef enum {
  LED_PANEL, LED_BUILTIN
} LED;

typedef enum {
  X1, X2, X3, X4, X5, X6, X7
} InputPort;

typedef enum {
  Q0, Q1, Q2, Q3, Led
} OutputPort;

void Test_Handle(Test_Type test);
void Test_Motor();
void Test_Encoder();
void Test_Input();
void Test_Output();
void Test_LCD();

void blink_output();

#endif
