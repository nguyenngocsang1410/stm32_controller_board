#include "test_handler.h"

#define X0 X0_GPIO_Port, X0_Pin
#define X1 X1_GPIO_Port, X1_Pin
#define X2 X2_GPIO_Port, X3_Pin
#define X3 X3_GPIO_Port, X3_Pin
#define X4 X4_GPIO_Port, X4_Pin
#define X5 X5_GPIO_Port, X5_Pin
#define X6 X6_GPIO_Port, X6_Pin
#define X7 X7_GPIO_Port, X7_Pin

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

void Test_Input() {
  /*
   * Test X0 -> X7
   * Out to: Debug Monitor
   * 		 revert PC13
   **/
  Button x0, x1, x2, x6, x7;
  buttonInit(&x0, X0);
  buttonInit(&x1, X1);
  buttonInit(&x2, X2);
  buttonInit(&x6, X6);
  buttonInit(&x7, X7);

}
void Test_Output() {
  OutputPort t = Q0; //Q0, Q1, Q2, Q3, LED_BUILTIN

  switch (t) {
	case Led:
	  break;
	case Q0:

	  break;
	case Q1:

	  break;
	case Q2:

	  break;
	case Q3:

	  break;
	default:
	  break;
  }
}
void Test_Motor() {

}
void Test_Encoder() {

}

void Test_LCD() {

}

/*=========================================*/

void blink_output() {

}
