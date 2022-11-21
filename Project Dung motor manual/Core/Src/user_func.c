/*
 * user_func.c
 *
 *  Created on: Sep 20, 2022
 *      Author: SangNguyen
 */

#include <stdbool.h>
#include <user_func.h>

extern bool END_REACH, BEGIN_REACH, MID_REACH;
uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax) {
  uint32_t val = au32_IN - au32_INmin;
  val = val * (au32_OUTmax - au32_OUTmin);
  val = val / (1.0 * (au32_INmax - au32_INmin));
  val = val + au32_OUTmin;
  return val;
}

void set_motor(u8 id, s8 dir, u16 val) {
  u16 pwm;
  switch (id) {
  case 1:
	pwm = MAP(val, 0, 1000, 0, 499);
	if (dir == 1) {
	  HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_SET);
	  TIM3->CCR1 = pwm;
	  TIM3->CCR2 = 0;
	} else if (dir == -1) {
	  HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_SET);
	  TIM3->CCR2 = pwm;
	  TIM3->CCR1 = 0;
	} else if (dir == -2) //Freely run
		{
	  HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_RESET);
	  TIM3->CCR1 = 0;
	  TIM3->CCR2 = 0;
	} else	// BRAKE
	{
	  HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_SET);
	  TIM3->CCR2 = 0;
	  TIM3->CCR1 = 0;
	}
	break;
  case 2:
	pwm = MAP(val, 0, 1000, 0, 999);
	if (dir == -1) {
	  HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
	  TIM4->CCR2 = pwm;
	} else if (dir == 1) {
	  HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_SET);
	  TIM4->CCR2 = pwm;
	} else if (dir == -2) //Freely run
		{
	  TIM4->CCR2 = 0;
	  HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
	} else //BRAKE
	{
	  HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
	  TIM4->CCR2 = 999;
	}
	break;
  default:
	return;
  }
}

void startup() {
  ;
}

void lightLED(bool yellow, bool green) {
  if (yellow) {
	HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
  } else if (yellow == false) {
	HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_RESET);
  }

  if (green) {
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
  } else if (green == false) {
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
  }
}

void returnHome() {
  while (!BEGIN_REACH) {
	set_motor_speed(2, -60);
  }
}

void set_motor_speed(u8 id, u8 rpm) {
  static u32 previous_milis = 0;
  static u32 preCounter1 = 0, preCounter2 = 0;
  u32 now = HAL_GetTick();

  if (now - previous_milis > 100) {

  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == LMS_B_Pin) {
	if (HAL_GPIO_ReadPin(LMS_B_GPIO_Port, LMS_B_Pin) == GPIO_PIN_SET) {
	  BEGIN_REACH = false;
	} else {
	  BEGIN_REACH = true;
	}
  } else if (GPIO_Pin == LMS_E_Pin) {
	if (HAL_GPIO_ReadPin(LMS_E_GPIO_Port, LMS_E_Pin) == GPIO_PIN_SET) {
	  END_REACH = false;
	} else {
	  END_REACH = true;
	}
  } else if (GPIO_Pin == LMS_M_Pin) {
	if (HAL_GPIO_ReadPin(LMS_M_GPIO_Port, LMS_M_Pin) == GPIO_PIN_SET) {
	  MID_REACH = false;
	} else {
	  MID_REACH = true;
	}
  } else
	return;
}
