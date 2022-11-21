/*
 * user_func.c
 *
 *  Created on: Sep 5, 2022
 *      Author: SangNguyen
 */

#include "user_func.h"
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define DEFAULT_MOTOR_SPD AD_RES //500

/* Private variables ---------------------------------------------------------*/
bool RUN_FLAG = false, IDLE_FLAG = false;
bool END_REACH = false, BEGIN_REACH = false, SP_REACH = false;

uint16_t pwm_val;
uint16_t AD_RES = 0;

/* Private user code ---------------------------------------------------------*/
uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax) {
  uint32_t val = au32_IN - au32_INmin;
  val = val * (au32_OUTmax - au32_OUTmin);
  val = val / (1.0 * (au32_INmax - au32_INmin));
  val = val + au32_OUTmin;
  return val;
}

void active_idle() {
  stop_motor();
  if (RUN_FLAG)
	RUN_FLAG = false;
  IDLE_FLAG = true;
}

void active_run() {
  stop_motor();
  if (IDLE_FLAG)
	IDLE_FLAG = false;
  RUN_FLAG = true;
}

void apply_state() {
  if (IDLE_FLAG) {
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

	check_setting();

	ButtonState BTN_START_curState = buttonRead(&BTN_START);
	ButtonState BTN_STOP_curState = buttonRead(&BTN_STOP);

	if (BTN_START_curState == 1 && BTN_STOP_curState == 1) {
	  set_motor(2, 0, 0);
	  return;
	} else if (BTN_START_curState == 0) {
	  if (END_REACH) {
		set_motor(2, 0, 0);
		return;
	  } else
		set_motor(2, 1, MAP(DEFAULT_MOTOR_SPD, 0, 4096, 0, 1000));
	} else if (BTN_STOP_curState == 0) {
	  if (BEGIN_REACH) {
		set_motor(2, 0, 0);
		return;
	  } else
		set_motor(2, -1, MAP(DEFAULT_MOTOR_SPD, 0, 4096, 0, 1000));
	} else {
	  set_motor(2, 0, 0);
	}
  } else if (RUN_FLAG) {
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

	ButtonState BTN_START_curState = buttonRead(&BTN_START);
	ButtonState BTN_STOP_curState = buttonRead(&BTN_STOP);

	if (BTN_START_curState == 1 && BTN_STOP_curState == 1) {
	  set_motor(1, 0, 0);
	  return;
	} else if (BTN_START_curState == 0) {
	  if (END_REACH) {
		set_motor(1, 0, 0);
		return;
	  } else
		set_motor(1, 1, MAP(DEFAULT_MOTOR_SPD, 0, 4096, 0, 1000));
	} else if (BTN_STOP_curState == 0) {
	  if (BEGIN_REACH) {
		set_motor(1, 0, 0);
		return;
	  } else
		set_motor(1, -1, MAP(DEFAULT_MOTOR_SPD, 0, 4096, 0, 1000));
	} else {
	  set_motor(1, 0, 0);
	}
  }
}

void check_setting() {

}

void set_motor(uint8_t id, int8_t dir, uint16_t pwm) {
  switch (id) {
  case 1: {
	uint32_t P_pwm = MAP(pwm, 0, 1000, 0, 499);
	if (dir == 1) {
	  TIM4->CCR3 = 0;
	  TIM4->CCR2 = P_pwm;
	} else if (dir == -1) {
	  TIM4->CCR2 = 0;
	  TIM4->CCR3 = P_pwm;
	} else {
	  TIM4->CCR2 = 0;
	  TIM4->CCR3 = 0;
	}
	break;
  }
  case 2: {
	set_motor2_dir(dir);
	uint32_t P_pwm = MAP(pwm, 0, 1000, 0, 999);
	TIM3->CCR2 = P_pwm;
	break;
  }
  }
}

void set_motor2_dir(int8_t dir) {
  if (dir == 1) {
	HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
  } else if (dir == -1) {
	HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_SET);
  } else {
	HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
  }
}

void stop_motor() {
  set_motor(1, 0, 0);
  set_motor(2, 0, 0);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
// Read & Update The ADC Result
  AD_RES = HAL_ADC_GetValue(&hadc1);
}
/**
 * Timer4 clock: 1MHz
 * overflow: 999 ticks -> 1ms -> 1kHz
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  if (GPIO_Pin == LMS1_Pin) {
	if (HAL_GPIO_ReadPin(LMS1_GPIO_Port, LMS1_Pin) == GPIO_PIN_SET) {
	  BEGIN_REACH = false;
	} else {
	  BEGIN_REACH = true;
	}
  } else if (GPIO_Pin == LMS2_Pin) {
	if (HAL_GPIO_ReadPin(LMS2_GPIO_Port, LMS2_Pin) == GPIO_PIN_SET) {
	  END_REACH = false;
	} else {
	  END_REACH = true;
	}
  } else if (GPIO_Pin == LMS_m_Pin) {
	if (HAL_GPIO_ReadPin(LMS_m_GPIO_Port, LMS_m_Pin) == GPIO_PIN_SET) {
	  SP_REACH = false;
	} else {
	  SP_REACH = true;
	}
  } else
	return;
}

