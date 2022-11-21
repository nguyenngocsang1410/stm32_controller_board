#include "test_handler.h"

#define X1 X1_GPIO_Port, X1_Pin

void Test_Handle(Test_Type test) {
  switch (test) {
  case MOTOR_TEST:
	Test_Motor();
	break;
  case ENCODER_TEST:
	Test_Encoder();
	break;
  case INPUT_TEST:
	Test_Input();
	break;
  case OUTPUT_TEST:
	Test_Output();
	break;
  case LCD_TEST:
	Test_LCD();
	break;
  default:
	break;
  }
}

void Test_Input(){
  /*
   * Test X0 -> X7
   * Out to: Debug Monitor
   * 		 revert PC13
   **/
  HAL_GPIO_TogglePin(X1);

}
void Test_Output(){

}
void Test_Motor(){

}
void Test_Encoder(){

}

void Test_LCD(){

}
