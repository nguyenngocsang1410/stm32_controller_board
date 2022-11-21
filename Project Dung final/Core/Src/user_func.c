/*
 * user_func.c
 *
 *  Created on: Sep 5, 2022
 *      Author: SangNguyen
 */

#include "user_func.h"
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define DEFAULT_MOTOR_SPD 4000

/* Private variables ---------------------------------------------------------*/
bool RUN_MODE = false, IDLE_MODE = false;
bool END_REACH = false, BEGIN_REACH = false, MID_REACH = false;

uint16_t pwm_val;
uint16_t AD_RES = 0;

uint8_t revLevel = 0;
LedOut LED_Y = { LED_Y_GPIO_Port, LED_Y_Pin };
LedOut LED_G = { LED_G_GPIO_Port, LED_G_Pin };

uint16_t numberOfRound[3] = { 100, 200, 300 };
float m1_velo = 0, m2_velo = 0;
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
  if (RUN_MODE)
	RUN_MODE = false;
  IDLE_MODE = true;
}
bool RUN_CMD = false;
bool M2_FWD = true;
uint16_t revCount = 0;
void active_run() {
  stop_motor();
  if (IDLE_MODE)
	IDLE_MODE = false;
  RUN_MODE = true;

  RUN_CMD = false;
}

void apply_state() {
  if (check_setting()) {
	stop_motor();
	if (RUN_MODE) {
	  active_run();
	}
  }

  if (IDLE_MODE) {
	HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);

	ButtonState BTN_START_curState = buttonRead(&BTN_START);
	ButtonState BTN_STOP_curState = buttonRead(&BTN_STOP);

	if ((BTN_START_curState == 1 && BTN_STOP_curState == 1)
		|| (BTN_START_curState == 0 && BTN_STOP_curState == 0)) {
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
	}
  } else if (RUN_MODE) {
	ButtonState BTN_START_curState = buttonRead(&BTN_START);
	ButtonState BTN_STOP_curState = buttonRead(&BTN_STOP);

	if (BTN_STOP_curState == 0) {
	  active_run();
	  return;
	}

	if (BEGIN_REACH == false && RUN_CMD == false) {
	  // Not HOME
	  flash_LED(LED_Y);
	  return;
	} else if (BEGIN_REACH == true && RUN_CMD == false) {
	  // At HOME, wait for command
	  if (BTN_START_curState == BTN_RESET) {
		RUN_CMD = true;
		M2_FWD = true;
		revCount = 0;
	  }
	}
	set_motor(1,1,500);
	HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);

	if (RUN_CMD) {
	  if (END_REACH) {
		M2_FWD = false;
	  }
	  if (MID_REACH) {
		if (M2_FWD == false)
		  revCount += 1;
		if (revCount == numberOfRound[revLevel]) {
		  active_run();
		  return;
		}
		M2_FWD = true;
	  }

	  if (M2_FWD) {
		set_motor(2, 1, 700);
	  } else {
		set_motor(2, -1, 700);
	  }
	}
  }
}

bool check_setting() {
  ButtonState lvl1, lvl2;
  lvl1 = buttonRead(&LVL1);
  lvl2 = buttonRead(&LVL2);
  uint8_t newLevel;
  if (lvl1 == BTN_SET && lvl2 == BTN_SET) // Middle pos
	  {
	newLevel = 1;
  } else if (lvl1 == BTN_RESET && lvl2 == BTN_SET) // Left pos
	  {
	newLevel = 0;
  } else if (lvl1 == BTN_SET && lvl2 == BTN_RESET) // Right pos
	  {
	newLevel = 2;
  }
  if (newLevel != revLevel) {
	revLevel = newLevel;
	return 1; // CHANGE
  } else
	return 0;
}

void set_motor(uint8_t id, int8_t dir, uint16_t pwm) {
  switch (id) {
  case 1: {
	uint32_t P_pwm = MAP(pwm, 0, 1000, 0, 499);
	if (dir == 1) {
	  TIM3->CCR2 = 0;
	  TIM3->CCR1 = P_pwm;
	} else if (dir == -1) {
	  TIM3->CCR1 = 0;
	  TIM3->CCR2 = P_pwm;
	} else {
	  TIM3->CCR1 = 0;
	  TIM3->CCR2 = 0;
	}
	break;
  }
  case 2: {
	set_motor2_dir(dir);
	uint32_t P_pwm = MAP(pwm, 0, 1000, 0, 999);
	TIM4->CCR2 = P_pwm;
	break;
  }
  }
}

void set_motor2_dir(int8_t dir) {
  if (dir == -1 && BEGIN_REACH == false) {
	HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
  } else if (dir == 1 && END_REACH == false) {
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

uint32_t last_led_Y = 0, last_led_G = 0;
#define FLASH_INTERVAL 1000 //ms
void flash_LED(LedOut led) {
  uint32_t now = HAL_GetTick();
  switch (led.Pin) {
  case LED_G_Pin:
	if (now - last_led_G > FLASH_INTERVAL) {
	  last_led_G = now;
	  HAL_GPIO_TogglePin(led.Port, led.Pin);
	}
	break;
  case LED_Y_Pin:
	if (now - last_led_Y > FLASH_INTERVAL) {
	  last_led_Y = now;
	  HAL_GPIO_TogglePin(led.Port, led.Pin);
	}
	break;
  }
}

int32_t pre_E1 = 0, pre_E2 = 0;
#define M1_GEAR_RATIO 116
#define M1_ENCODER_PULSE 1600

#define M2_GEAR_RATIO 171
#define M2_ENCODER_PULSE 44
/**
 * Timer4 clock: 1MHz
 * overflow: 999 ticks -> 1ms -> 1kHz
 */
int32_t now_E1;
int32_t now_E2;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  now_E1 = (int32_t) TIM1->CNT;
  now_E2 = (int32_t) TIM2->CNT;

  int32_t delta = now_E1 - pre_E1;

//velo = delta * 60 sec * 1000 ms / (ECD pulse per round * gear ratio)
  m1_velo = delta * 1.0 * 60 * 1000 / (M1_ENCODER_PULSE * M1_GEAR_RATIO);
  delta = now_E2 - pre_E2;
  m2_velo = delta * 1.0 * 60 * 1000 / (M2_ENCODER_PULSE * M2_GEAR_RATIO);
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

