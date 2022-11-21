/*
 * user_func.c (FINAL x3)
 *
 *  Created on: Sep 20, 2022
 *      Author: SangNguyen
 */

#include <user_func.h>
bool RUN_MODE = false, IDLE_MODE = false;

ButtonState BTN_Start_state = BTN_SET, BTN_Stop_state = BTN_SET;
ButtonState SW_Lvl1_state = BTN_SET, SW_Lvl2_state = BTN_SET;
u8 level = 0;

enum {
  LED_Y = 0, LED_G = 1
};
bool FLAG_returnHome = false;

float distanceToGo = 0;
s8 M2_dir = 0;
#define travelLength 250

u32 tick_count = 0; // 1 ms/tick
#define M1_PulsePerRev 1600	// tick/motor rev
#define M1_GearRatio (30.0/16.0)	// motor rev / output rev

#define M2_PulsePerRev 16*4	// tick/motor rev
#define M2_GearRatio 30		// motor rev / output rev
#define M2_visme_pitch	2	// translation mm/ rev

float M1_velo = 0, M2_velo = 0;
s32 now_E1;
s32 now_E2;

u32 pre_E1 = 0;
u32 pre_E2 = 0;

float M2_x = 0;
float M2_travel = 0;
float M1_travel = 0;
bool FLAG_returnFast = false;
bool FLAG_returnFine = false;
u32 TS_returnFine = 0;

float M2_travel_StartPoint = 0;
bool FLAG_runMotor = false;

float M1_vref = 0, M2_vref = 0;
float M1_velo_filt = 0, M2_velo_filt = 0;
float M1_preVelo = 0, M2_preVelo = 0;
float M1_velo_eint = 0, M2_velo_eint = 0;
float M1_Kp = 1.84, M1_Ki = 184.03, M1_Kd = 0;
float M2_Kp = 1, M2_Ki = 0;
s8 M1_dir = 0;
float M1_u = 0, M2_u = 0;
float M1_velo_e = 0, M2_velo_e = 0;
float M1_velo_preE = 0;
u16 M1_toRotate = 0;
bool FLAG_completeRun = false;
u8 preLevel = 0;

/*===================================================*/
uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax) {
  uint32_t val = au32_IN - au32_INmin;
  val = val * (au32_OUTmax - au32_OUTmin);
  val = val / (1.0 * (au32_INmax - au32_INmin));
  val = val + au32_OUTmin;
  return val;
}

void set_motor_velo(u8 id, s16 val) {
  switch (id) {
  case 1:
	M1_vref = val;
	break;
  case 2:
	M2_vref = val;
	break;
  }
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
	} else if (dir == 0) //Freely run
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
	if (dir == -1 && BEGIN_REACH == false) {
	  HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
	  TIM4->CCR2 = pwm;
	} else if (dir == 1 && END_REACH == false) {
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
bool FLAG_startPointSet = false;
bool FLAG_midInTime = false;
u32 midInTime = 0;
u32 midDelta = 0;
bool FLAG_midOutTime = false;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == LMS_B_Pin) {
	if (HAL_GPIO_ReadPin(LMS_B_GPIO_Port, LMS_B_Pin) == GPIO_PIN_SET) {
	  BEGIN_REACH = false;
	} else {
	  BEGIN_REACH = true;
	  if (M2_dir == -1)
		set_motor(2, 0, 0);
	}
  } else if (GPIO_Pin == LMS_E_Pin) {
	if (HAL_GPIO_ReadPin(LMS_E_GPIO_Port, LMS_E_Pin) == GPIO_PIN_SET) {
	  END_REACH = false;
	} else {
	  END_REACH = true;
	  if (M2_dir == 1)
		set_motor(2, 0, 0);
	}
  } else if (GPIO_Pin == LMS_M_Pin) {
	if (HAL_GPIO_ReadPin(LMS_M_GPIO_Port, LMS_M_Pin) == GPIO_PIN_SET) {
	  MID_REACH = false;
	  FLAG_midInTime = false;
	  if (FLAG_midOutTime == false) {
		FLAG_midOutTime = true;
		u32 now_midOut = HAL_GetTick();
		midDelta = now_midOut - midInTime;
		midInTime = now_midOut;
		if (midDelta > 3000) {
		  FLAG_startPointSet = false;
		  midDelta = 0;
		}

		if ((FLAG_startPointSet == false) && (FLAG_runMotor == true)) {
		  M2_travel_StartPoint = M2_x;
		  M2_travel = 0;
		  M1_travel = 0;
		  FLAG_startPointSet = true;
		}
	  }
	} else {
	  MID_REACH = true;
	  FLAG_midOutTime = false;
	  if (FLAG_midInTime == false) {
		FLAG_midInTime = true;
		midInTime = HAL_GetTick();
	  }
	}
  } else
	return;
}

void start_up() {
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  HAL_TIM_Base_Start_IT(&htim4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  buttonInit(&BTN_Start, BTN_START_GPIO_Port, BTN_START_Pin);
  buttonInit(&BTN_Stop, BTN_STOP_GPIO_Port, BTN_STOP_Pin);
  buttonInit(&SW_Mode, MODE_SW_GPIO_Port, MODE_SW_Pin);
  buttonInit(&SW_Lvl1, LVL_SW1_GPIO_Port, LVL_SW1_Pin);
  buttonInit(&SW_Lvl2, LVL_SW2_GPIO_Port, LVL_SW2_Pin);

  state = buttonRead(&SW_Mode);
  if (state == IDLE)
	active_idle();
  else if (state == RUN)
	active_run();

  if (HAL_GPIO_ReadPin(LMS_B_GPIO_Port, LMS_B_Pin) == GPIO_PIN_RESET) {
	BEGIN_REACH = true;
  }
  if (HAL_GPIO_ReadPin(LMS_E_GPIO_Port, LMS_E_Pin) == GPIO_PIN_RESET) {
	END_REACH = true;
  }
  if (HAL_GPIO_ReadPin(LMS_M_GPIO_Port, LMS_M_Pin) == GPIO_PIN_RESET) {
	MID_REACH = true;
  }
}
void stop_motor(bool motor1, bool motor2) {
  if (motor1) {
	set_motor(1, 0, 0);
  }
  if (motor2) {
	set_motor(2, 0, 0);
  }
}
void active_idle() {
  stop_motor(1, 1);
  if (RUN_MODE == true) {
	RUN_MODE = false;
  }
  IDLE_MODE = true;
  lightLED(1, 0);
  FLAG_returnHome = false;
}
void active_run() {
  stop_motor(1, 1);
  if (IDLE_MODE == true) {
	IDLE_MODE = false;
  }
  RUN_MODE = true;
  lightLED(0, 1);
  FLAG_runMotor = false;

  FLAG_completeRun = false;
  preLevel = level;
}

void check_button() {
  BTN_Start_state = buttonRead(&BTN_Start);
  BTN_Stop_state = buttonRead(&BTN_Stop);

  SW_Lvl1_state = buttonRead(&SW_Lvl1);
  SW_Lvl2_state = buttonRead(&SW_Lvl2);

  if (SW_Lvl1_state == BTN_SET && SW_Lvl2_state == BTN_SET) {
	level = 1;
  } else if (SW_Lvl1_state == BTN_RESET) {
	level = 0;
  } else if (SW_Lvl2_state == BTN_RESET) {
	level = 2;
  }
}

void flash_LED(u8 id, u16 interval) {
  static u32 last_Y = 0, last_G = 0;
  u32 now = HAL_GetTick();
  switch (id) {
  case LED_Y:
	if ((now - last_Y) > interval) {
	  HAL_GPIO_TogglePin(LED_Y_GPIO_Port, LED_Y_Pin);
	  last_Y = now;
	}
	break;
  case LED_G:
	if ((now - last_G) > interval) {
	  HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
	  last_G = now;
	}
	break;
  }
}
bool FLAG_returnFineRev = false;
void return_home() {
  if (FLAG_returnFast) {
	if (BEGIN_REACH) {
	  FLAG_returnFast = false;
	  FLAG_returnFine = true;
	  TS_returnFine = HAL_GetTick();
	  set_motor(2, 0, 0);
	  return;
	}
	set_motor(2, -1, 1000);
	FLAG_returnFine = false;
	FLAG_returnFineRev = false;
  }
  if (FLAG_returnFine) {
	u32 deltaTime = HAL_GetTick() - TS_returnFine;

	if (deltaTime > 2000) {
	  FLAG_returnFineRev = true;
	  FLAG_returnFine = false;
	  set_motor(2, 0, 0);
	  return;
	}
	set_motor(2, 1, 1000);
  }
  if (FLAG_returnFineRev) {
	if (BEGIN_REACH) {
	  set_motor(2, 0, 0);
	  FLAG_returnHome = false;
	  FLAG_startPointSet = false;
	  HAL_NVIC_DisableIRQ(TIM4_IRQn);
	  M2_x = 0;
	  M2_travel = 0;
	  HAL_NVIC_EnableIRQ(TIM4_IRQn);
	  return;
	}
	set_motor(2, -1, 800);
  }
}

s32 delta_E1 = 0, delta_E2 = 0;
u8 deltaT = 20; //ms
float M1_pwm = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
	tick_count = tick_count + 1;
	if (tick_count >= deltaT) {
	  //50 ms
	  tick_count = 0;
	  {
		now_E1 = TIM1->CNT;
		now_E2 = TIM2->CNT;

		s32 delta_E1 = now_E1 - pre_E1;
		s32 delta_E2 = now_E2 - pre_E2;

		if (delta_E1 < -3000) // now = 5, pre = 65530
		  delta_E1 = delta_E1 + 0xFFFF;
		else if (delta_E1 > 3000) // now = 65535, pre = 5
		  delta_E1 = delta_E1 - 0xFFFF;

		if (delta_E2 < -3000) // now = 5, pre = 65530
		  delta_E2 = delta_E2 + 0xFFFF;
		else if (delta_E2 > 3000) // now = 65535, pre = 5
		  delta_E2 = delta_E2 - 0xFFFF;

		// (ticks/s*60)/(ticks/mRev*mRev/outRev) = outRev/min
		M1_velo = (delta_E1 * (1000 / deltaT) * 60.0)
			/ (1.0 * M1_PulsePerRev * M1_GearRatio);
		M2_velo = (delta_E2 * (1000 / deltaT) * 60.0)
			/ (1.0 * M2_PulsePerRev * M2_GearRatio);

		// pulse/(pulse/rev*rev/Outrev) = OutRev *pitch = mm
		M2_x = M2_x
			+ delta_E2 * M2_visme_pitch * 1.0
				/ (1.0 * M2_PulsePerRev * M2_GearRatio);

		pre_E1 = now_E1;
		pre_E2 = now_E2;

		u32 delta_E2_abs = abs(delta_E2);
		M2_travel = M2_travel
			+ (delta_E2_abs * M2_visme_pitch * 1.0)
				/ (1.0 * M2_PulsePerRev * M2_GearRatio);
		M1_travel = M1_travel
			+ delta_E1 * 1.0 / (1.0 * M1_PulsePerRev * M1_GearRatio);

		M1_velo_filt = -0.222 * M1_velo_filt + 0.611 * M1_velo
			+ 0.611 * M1_preVelo;
		M1_preVelo = M1_velo;

		M1_velo_e = M1_vref - M1_velo_filt;
		M1_velo_eint = M1_velo_eint + M1_velo_e * deltaT / 1000.0;
		M1_u = M1_Kp * M1_velo_e + M1_Ki * M1_velo_eint;
		M1_velo_preE = M1_velo_e;
		M1_pwm = fabsf(M1_u);
		if (M1_pwm > 1000.0)
		  M1_pwm = 1000.0;
		else if (M1_pwm < 0)
		  M1_pwm = 0;

		if (M1_u < 0)
		  M1_dir = 1;
		else
		  M1_dir = -1;

		if (FLAG_runMotor) {
		  set_motor(1, M1_dir, M1_pwm);
		} else
		  set_motor(1, 0, 0);
	  }
	}
  }
}

void run_motor() {

  if (M1_travel >= M1_toRotate) {
	set_motor(2, 0, 0);
	set_motor_velo(1, 0);
	set_motor(1, 0, 0);
	flash_LED(LED_G, 300);
	FLAG_completeRun = true;
	FLAG_runMotor = false;
	return;
  } else if (M2_dir == 1) {
	set_motor(2, M2_dir, 1000);
  } else if (M2_dir == -1) {
	set_motor(2, M2_dir, 1000);
  }
  set_motor_velo(1, 40);
  if (M2_dir == 1 && M2_x >= travelLength) {
	M2_dir = -1;
  } else if (M2_dir == -1 && M2_x <= (M2_travel_StartPoint + 5)) {
	M2_dir = 1;
  }
}

bool FLAG_notHome = false;
void apply_state() {
  check_button();
  if (IDLE_MODE) {
	if (BTN_Stop_state == BTN_RESET) {
	  FLAG_returnHome = false;
	  set_motor(2, 0, 0);
	}
	if (BTN_Start_state == BTN_RESET && BEGIN_REACH == false) {
	  FLAG_returnHome = true;
	  FLAG_returnFast = true;
	}

	if (FLAG_returnHome) {
	  return_home();
	}
  } else if (RUN_MODE) {
	if (preLevel != level) {
	  active_run();
	}
	FLAG_returnHome = false;

	if (level != 0 && FLAG_runMotor == true) {
	  run_motor();
	  lightLED(0, 1);
	} else if (level != 0 && !FLAG_runMotor)
	  stop_motor(1, 1);
	if (level != 0 && BTN_Stop_state == BTN_RESET) {
	  FLAG_runMotor = false;
	  FLAG_completeRun = false;
	  stop_motor(1, 1);
	}

	if (FLAG_completeRun == true) {
	  flash_LED(LED_G, 800);
	  return;
	}
	if (!FLAG_notHome) {
	  if (level == 1) {
		distanceToGo = 2000;
		M1_toRotate = 42;
	  } else if (level == 2) {
		distanceToGo = 4000;
		M1_toRotate = 84;
	  }
	  if (BTN_Stop_state == BTN_RESET) {
		FLAG_runMotor = false;
//		stop_motor(1, 1);
	  } else if (BTN_Start_state == BTN_RESET && !FLAG_runMotor) {
		FLAG_runMotor = true;
		M2_travel_StartPoint = 40;
		M2_dir = 1;
		M1_travel = 0;
		M1_velo_eint = 0;
	  }
	}

	switch (level) {
	case 0:
	  lightLED(0, 1);
	  if (BTN_Start_state == BTN_RESET && END_REACH == false) {
		set_motor(2, 1, 1000);
	  } else if (BTN_Stop_state == BTN_RESET && BEGIN_REACH == false) {
		set_motor(2, -1, 1000);
	  } else {
		set_motor(2, 0, 0);
	  }
	  return;
	  break;
	case 1:
	case 2:
	  if (!BEGIN_REACH && !FLAG_runMotor) {
		flash_LED(LED_G, 500);
		FLAG_notHome = true;
		return;
	  } else if (BEGIN_REACH && !FLAG_runMotor) {
		FLAG_notHome = false;
		lightLED(0, 1);
	  }
	  break;
	}
  }
}

