#ifndef _TEST_HANDLER_H_
#define _TEST_HANDLER_H_

#include "main.h"

typedef enum {
  MOTOR_TEST, ENCODER_TEST, INPUT_TEST, OUTPUT_TEST, LCD_TEST
} Test_Type;

void Test_Handle(Test_Type test);
void Test_Motor();
void Test_Encoder();
void Test_Input();
void Test_Output();
void Test_LCD();

#endif
