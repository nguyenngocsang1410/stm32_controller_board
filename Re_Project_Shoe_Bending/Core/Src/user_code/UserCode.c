#include "UserCode.h"
#include "stdlib.h"

CLCD_I2C_Name LCD;
Button BTN_Start, BTN_Stop, BTN_Mode;

float mSpeed = 0;
s8 mDir = 1;
s32 mNumLoopCount = 30;
u16 nLoop = 0;

u8 state = 1;
u8 preState = 1;
u8 menuCounter = 1; //counts the clicks of the rotary encoder between menu items (0-3 in this case)

bool menu1_selected = false; //enable/disable to change the value of menu item
bool menu2_selected = false;
bool menu3_selected = false;
bool menu4_selected = false;

bool refreshLCD = true; //refreshes values
bool refreshSelection = false; //refreshes selection (> / X)

u32 STOP_from = 0;
bool STOP_cmd = false;

float curSpeed = 0, preSpeed = 0;
u32 curPos = 0, prePos = 0;
u32 curTime = 0, preTime = 0;
u32 timeInterval = 20; //ms

float kp = 10, ki = 0, kd = 0;
float eint = 0, e = 0, preE = 0;
float vref = 0;
s32 pre_posi = 0;
s8 pidDir = 1;

bool FLAG_run = false;

u32 pwm = 500;

void start_up() {
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  HAL_TIM_Base_Start_IT(&htim4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);	//PWM1
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //PWM1
//  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);	//PWM2
//  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);	//PWM2
  HAL_Delay(10);

  buttonInit(&BTN_Start, BTN_Start_GPIO_Port, BTN_Start_Pin);
  buttonInit(&BTN_Stop, BTN_Stop_GPIO_Port, BTN_Stop_Pin);
  buttonInit(&BTN_Mode, BTN_Mode_GPIO_Port, BTN_Mode_Pin);

  // Splash screen
  LCD_Begin(&LCD, &hi2c2, 0x4E, 20, 4);
  /*******************
   ----------------------
   |   Taber Abrasion   |
   | Resistance Testing |
   |       Machine      |
   |                    |
   ----------------------
   **************************/
  LCD_Clear(&LCD);
  LED_ON();
  LCD_Print_String_At(&LCD, 1, 4, "Taber Abrasion");
  LCD_Print_String_At(&LCD, 2, 2, "Resistance Testing");
  LCD_Print_String_At(&LCD, 3, 8, "Machine");

  // Restore value
  menu1_value = (float) FLASH_ReadData(FLASH_USER_START_ADDR); // Speed
  menu2_value = (s8) FLASH_ReadData(FLASH_USER_START_ADDR + 4); // Dir
  menu3_value = (s32) FLASH_ReadData(FLASH_USER_START_ADDR + 8); //set count

  HAL_Delay(1000);
  LCD_Clear(&LCD);
  LED_OFF();
}

void check_state() {
  // If Enter new state
  if (preState != state) {
	LCD_Clear(&LCD);
	preState = state;
	refreshLCD = true;
	refreshSelection = true;
  }
}
void main_loop() {
  check_state();
  check_ecd();

  switch (state) {
	case 1: //Idle
	  /*******************
	   ----------------------
	   |Speed: 60rpm        |
	   |Direction: CW       |
	   |Count:    0 revs    |
	   |Set count: xxxx     |
	   ----------------------
	   **************************/
	{
	  if (refreshLCD == true) {
		printDefaultLCD();
		HAL_Delay(10);
		refreshLCD = false;
	  }
	  break;
	} //case 1
	case 2: //Choose para --> blink
	  /*******************
	   ----------------------
	   |>Speed: 60rpm       |
	   | Direction: CW      |
	   | Set count: xxxx    |
	   |                    |
	   ----------------------
	   **************************/
	{
	  if (refreshLCD == true) //If we are allowed to update the LCD ...
	  {
		printMenuName();
		printMenuValue(); // ... we update the LCD ...

		//... also, if one of the menus are already selected...
		if (menu1_selected == true || menu2_selected == true
			|| menu3_selected == true || menu4_selected == true) {
		  // do nothing
		} else {
		  updateCursorPosition(); //update the position
		}

		refreshLCD = false; //reset the variable - wait for a new trigger
	  }
	  if (refreshSelection == true) //if the selection is changed
	  {
		updateSelection(); //update the selection on the LCD
		refreshSelection = false; // reset the variable - wait for a new trigger
	  }
	  break;
	}
	case 3: // Run
	  /*******************
	   ----------------------
	   |Speed: 60rpm        |
	   |Direction: CW       |
	   |Set count: xxxx     |
	   |Count:    0 revs    |
	   ----------------------
	   **************************/
	{
	  printDefaultLCD();
	  s32 motor_posi = TIM1_count;
	  s32 delta = motor_posi - pre_posi;
	  if (abs(delta) > totalPulse) {
		nLoop++;
		pre_posi = motor_posi;
	  }

	  if (nLoop >= mNumLoopCount)
		nLoop = mNumLoopCount;
	  char holder[10];
	  LCD_Print_String_At(&LCD, 4, 8, "         ");
	  if (nLoop < 2) {
		sprintf(holder, "%4d rev", nLoop);
	  } else {
		sprintf(holder, "%4d revs", nLoop);
	  }
	  LCD_Print_String_At(&LCD, 4, 8, holder);

	  // Stop condition
	  if (nLoop >= mNumLoopCount) {
		state = 4;
		set_motor(1, -2, 0);
		LED_OFF();
	  }

	  //out to motor

//	  s32 mSpeedPwm = map(mSpeed,minSpeed,maxSpeed,0,1000);
	  s32 mSpeedPwm = 700;
	  if (mDir == 1) {
		set_motor(1, 1, pwm);
	  } else {
		set_motor(1, -1, pwm);
	  }
	  break;
	}
	case 4: //Result view
	  /*******************
	   ----------------------
	   |Speed: 60rpm        |
	   |Direction: CW       |
	   |Count:    0 revs    |
	   |Set count: xxxx     |
	   ----------------------
	   **************************/
	{
	  FLAG_run = false;
	  set_motor(1, -2, 0);
	  LED_TOGGLE();
	  printDefaultLCD();

	  HAL_Delay(300);
	  break;
	}
	default:
	  return;
  } //Switch lvl1
}

void LED_ON() {
  HAL_GPIO_WritePin(LED_Panel_GPIO_Port, LED_Panel_Pin, GPIO_PIN_SET);
}
void LED_OFF() {
  HAL_GPIO_WritePin(LED_Panel_GPIO_Port, LED_Panel_Pin, GPIO_PIN_RESET);
}
void LED_TOGGLE() {
  HAL_GPIO_TogglePin(LED_Panel_GPIO_Port, LED_Panel_Pin);
}

void FLASH_WritePage(uint32_t startPage, uint32_t endPage, uint32_t data) {
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef EraseInit;
  EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInit.PageAddress = startPage;
  EraseInit.NbPages = (endPage - startPage) / FLASH_PAGE_SIZE;
  uint32_t PageError = 0;
  HAL_FLASHEx_Erase(&EraseInit, &PageError);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startPage, data);
  HAL_FLASH_Lock();
}
uint32_t FLASH_ReadData(uint32_t addr) {
  /*
   * Read 4 bytes
   * */
  uint32_t data = *(__IO uint32_t*) (addr);
  return data;
}

void printMenuName() {
  /*******************
   ----------------------
   |>Speed: 60rpm       |
   | Direction: CW      |
   | Set count: xxxx    |
   |                    |
   ----------------------
   **************************/

  //These are the values which are not changing the operation
  LCD_Print_String_At(&LCD, 1, 2, "Speed: ");
  //----------------------
  LCD_Print_String_At(&LCD, 2, 2, "Direction: ");
  LCD_Print_String_At(&LCD, 3, 2, "Set count: ");
  //----------------------
}
void printMenuValue() {
  /*******************
   ----------------------
   |>Speed: 60rpm       |
   | Direction: CW      |
   | Set count: xxxx    |
   |                    |
   ----------------------
   **************************/
  //Update value
  LCD_Print_String_At(&LCD, 1, 9, "      ");
  char holder[10];
  sprintf(holder, "%3d rpm", (int) menu1_value);
  LCD_Print_String_At(&LCD, 1, 9, holder);

  LCD_Print_String_At(&LCD, 2, 13, "    ");
  if (menu2_value == 1) {
	sprintf(holder, " CW");
  } else {
	sprintf(holder, "CCW");
  }
  LCD_Print_String_At(&LCD, 2, 13, holder);

  LCD_Print_String_At(&LCD, 3, 13, "        ");
  sprintf(holder, "%3d revs", (int) menu3_value);
  LCD_Print_String_At(&LCD, 3, 13, holder);
}

void updateCursorPosition() {
  //Clear display's ">" parts
  LCD_Print_String_At(&LCD, 1, 1, " ");
  LCD_Print_String_At(&LCD, 2, 1, " ");
  LCD_Print_String_At(&LCD, 3, 1, " ");
  LCD_Print_String_At(&LCD, 4, 1, " ");

  //Place cursor to the new position
  switch (menuCounter) //this checks the value of the counter (0, 1, 2 or 3)
  {
	case 1:
	  LCD_Print_String_At(&LCD, 1, 1, ">");
	  break;
	  //-------------------------------
	case 2:
	  LCD_Print_String_At(&LCD, 2, 1, ">");
	  break;
	  //-------------------------------
	case 3:
	  LCD_Print_String_At(&LCD, 3, 1, ">");
	  break;
	  //-------------------------------
	case 4:
	  LCD_Print_String_At(&LCD, 4, 1, ">");
	  break;
  }
}
void updateSelection() {
  //When a menu is selected ">" becomes "X"

  if (menu1_selected == true) {
	LCD_Print_String_At(&LCD, 1, 1, "*");
  }
  //-------------------
  if (menu2_selected == true) {
	LCD_Print_String_At(&LCD, 2, 1, "*");
  }
  //-------------------
  if (menu3_selected == true) {
	LCD_Print_String_At(&LCD, 3, 1, "*");
  }
  //-------------------
  if (menu4_selected == true) {
	LCD_Print_String_At(&LCD, 4, 1, "*");
  }
}

void printDefaultLCD() {
  /*******************
   ----------------------
   |Speed: 60rpm        |
   |Direction: CW       |
   |Set count: xxxx     |
   |Count:    0 revs    |
   ----------------------
   **************************/
  LCD_Print_String_At(&LCD, 1, 1, "Speed: ");
  //----------------------
  LCD_Print_String_At(&LCD, 2, 1, "Direction: ");
  LCD_Print_String_At(&LCD, 3, 1, "Set count: ");
  LCD_Print_String_At(&LCD, 4, 1, "Count: ");

  //Update value
  LCD_Print_String_At(&LCD, 1, 8, "      ");
  char holder[10];
  sprintf(holder, "%2d rpm", (int) menu1_value);
  LCD_Print_String_At(&LCD, 1, 8, holder);

  LCD_Print_String_At(&LCD, 2, 12, "    ");
  if (menu2_value == 1) {
	sprintf(holder, " CW");
  } else {
	sprintf(holder, "CCW");
  }
  LCD_Print_String_At(&LCD, 2, 12, holder);

  LCD_Print_String_At(&LCD, 3, 12, "        ");

  sprintf(holder, "%3d revs", (int) menu3_value);
  LCD_Print_String_At(&LCD, 3, 12, holder);

  LCD_Print_String_At(&LCD, 4, 8, "         ");
  if (nLoop < 2) {
	sprintf(holder, "%4d rev", nLoop);
  } else {
	sprintf(holder, "%4d revs", nLoop);
  }
  LCD_Print_String_At(&LCD, 4, 8, holder);
}

void set_motor(u8 id, s8 dir, u16 val) {
  u16 pwm;
  switch (id) {
	case 1:	// BTS7960
	  pwm = map(val, 0, 1000, 0, 499);
	  if (dir == 1) {
		HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_SET);
		M1A_Channel = pwm;
		M1B_Channel = 0;
	  } else if (dir == -1) {
		HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_SET);
		M1B_Channel = pwm;
		M1A_Channel = 0;
	  } else if (dir == 0) //Freely run
		  {
		HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_RESET);
		M1A_Channel = 0;
		M1B_Channel = 0;
	  } else	// BRAKE
	  {
		HAL_GPIO_WritePin(M1_EN_GPIO_Port, M1_EN_Pin, GPIO_PIN_SET);
		M1B_Channel = 0;
		M1A_Channel = 0;
	  }
	  break;
	case 2:	// L298N
	  pwm = map(val, 0, 1000, 0, 999);
	  if (dir == -1) {
		HAL_GPIO_WritePin(M2B_L_GPIO_Port, M2B_L_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(M2B_R_GPIO_Port, M2B_R_Pin, GPIO_PIN_RESET);
		M2B_Channel = pwm;
	  } else if (dir == 1) {
		HAL_GPIO_WritePin(M2B_L_GPIO_Port, M2B_L_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(M2B_R_GPIO_Port, M2B_R_Pin, GPIO_PIN_SET);
		M2B_Channel = pwm;
	  } else if (dir == -2) //Freely run
		  {
		M2B_Channel = 0;
		HAL_GPIO_WritePin(M2B_L_GPIO_Port, M2B_L_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(M2B_R_GPIO_Port, M2B_R_Pin, GPIO_PIN_RESET);
	  } else //BRAKE dir == 0
	  {
		HAL_GPIO_WritePin(M2B_L_GPIO_Port, M2B_L_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(M2B_R_GPIO_Port, M2B_R_Pin, GPIO_PIN_RESET);
		M2B_Channel = 999;
	  }
	  break;
	default:
	  return;
  }
}

s32 map(s32 x, s32 in_min, s32 in_max, s32 out_min, s32 out_max) {
  return (s32) ((x - in_min) * (out_max - out_min) * 1.0 / (in_max - in_min)
	  + out_min);
}

void reset_state() {
  eint = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == BTN_Mode_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
  {
	if (HAL_GPIO_ReadPin(BTN_Mode_GPIO_Port, BTN_Mode_Pin) == GPIO_PIN_RESET) {
	  BTN_Mode.StartPress = HAL_GetTick(); //ms
	} else {
	  BTN_Mode.StopPress = HAL_GetTick();
	  BTN_Mode.timePress = BTN_Mode.StopPress - BTN_Mode.StartPress;
	  if (BTN_Mode.timePress > DEBOUND_TIME
		  && BTN_Mode.timePress <= SINGLE_CLICK_TIME) {
		if (state == 1 || state == 2) {
		  state = state + 1;
		  if (state > 2) {
			switch (menuCounter) {
			  case 1:
				menu1_selected = !menu1_selected; //we change the status of the variable to the opposite
				break;

			  case 2:
				menu2_selected = !menu2_selected;
				break;

			  case 3:
				menu3_selected = !menu3_selected;
				break;
			  default:
				break;
			}
			refreshSelection = true;
			refreshLCD = true;
			state = 2;
		  }
		}
	  } else if (BTN_Mode.timePress > SINGLE_CLICK_TIME) {
		if (state == 2) {
		  state = 1;
		  HAL_FLASH_Unlock();
		  FLASH_EraseInitTypeDef EraseInit;
		  EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		  EraseInit.PageAddress = FLASH_USER_START_ADDR;
		  EraseInit.NbPages = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR )
			  / FLASH_PAGE_SIZE;
		  uint32_t PageError = 0;
		  HAL_FLASHEx_Erase(&EraseInit, &PageError);
		  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR,
		  menu1_value);
		  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR + 4,
		  menu2_value);
		  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR + 8,
		  menu3_value);
		  HAL_FLASH_Lock();
		} else if (state >= 4) {
		  STOP_cmd = true;
		  STOP_from = HAL_GetTick();
		}
	  }
	}
  }

  if (GPIO_Pin == BTN_Start_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
  {
	if (HAL_GPIO_ReadPin(BTN_Mode_GPIO_Port, BTN_Start_Pin) == GPIO_PIN_RESET) {
	  if (state == 1) {
		state = 3;
		reset_state();

		if (mDir == 1)
		  set_motor(1, 1, pwm);
		else
		  set_motor(1, -1, pwm);
//		vref = mSpeed;
//		FLAG_run = true;
	  } else if (state == 2) {
		state = 1;
		HAL_FLASH_Unlock();
		FLASH_EraseInitTypeDef EraseInit;
		EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInit.PageAddress = FLASH_USER_START_ADDR;
		EraseInit.NbPages = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR )
			/ FLASH_PAGE_SIZE;
		uint32_t PageError = 0;
		HAL_FLASHEx_Erase(&EraseInit, &PageError);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR,
		menu1_value);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR + 4,
		menu2_value);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR + 8,
		menu3_value);
		HAL_FLASH_Lock();
	  }
	}
  }
  if (GPIO_Pin == BTN_Stop_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
  {
	if (HAL_GPIO_ReadPin(BTN_Mode_GPIO_Port, BTN_Stop_Pin) == GPIO_PIN_RESET) {
	  if (state == 3) {
		set_motor(1, -2, 0);
		state = 4;
	  } else if (state == 4) {
		nLoop = 0;
		state = 1;
	  }
	}
  }
}

void LED_Flash() {
  static u32 last_flash = 0;
  u32 now = HAL_GetTick();
  if (now - last_flash > 300) {
	LED_TOGGLE();
	last_flash = now;
  }
}

u32 preEcd = 0;
void check_ecd() {
  if (state == 2) {
	if (preEcd != TIM2_count) {
	  s32 delta = TIM2_count - preEcd;
	  if (abs(delta) > ecdFilter) {
		if (menu1_selected == true) {
		  if (delta > 0)
			menu1_value++;
		  else if (delta < 0)
			menu1_value--;

		  if (menu1_value > maxSpeed) //we do not go above 100
			menu1_value = maxSpeed;
		  if (menu1_value < minSpeed)
			menu1_value = minSpeed;
		} else if (menu2_selected == true) {
		  if (delta > 0)
			menu2_value++;
		  else if (delta < 0)
			menu2_value--;

		  if (menu2_value > 1)
			menu2_value = 1;
		  if (menu2_value < 0)
			menu2_value = 0;
		} else if (menu3_selected == true) {
		  if (delta > 0)
			menu3_value++;
		  else if (delta < 0)
			menu3_value--;

		  if (menu3_value > 999)
			menu3_value = 999;
		  if (menu3_value < 10)
			menu3_value = 10; //rpm

		} else if (state == 2) {
		  if (delta > 0)
			menuCounter++;
		  else if (delta < 0)
			if (menuCounter > 0)
			  menuCounter--;

		  if (menuCounter > 3)
			menuCounter = 3;
		  if (menuCounter < 1)
			menuCounter = 1; //rpm

		}
		refreshLCD = true;
		preEcd = TIM2_count;
	  }
	}
  } else {
	preEcd = TIM2_count;
  }
  return;
}

u32 timer_count = 0;
float u = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) // 1ms
  {
	timer_count++;
	if (timer_count >= timeInterval) // 20ms 50Hz
		{
	  curPos = TIM1_count;
	  s32 deltaPos = curPos - prePos;
	  prePos = curPos;
	  curSpeed = 60 * (fabs(deltaPos) / (totalPulse)) / (timeInterval / 1000.0);
	  e = curSpeed - vref;
	  eint = eint + e * timeInterval;
	  float de = (e - preE) / timeInterval;
	  preE = e;
	  u = kp * e + ki * eint + kd * de;   //%PWM

	  if (u < 0) {
		pidDir = -mDir;
		u = fabs(u);
	  }
	  if (u > 1000)
		u = 1000;

	  if (FLAG_run == true) {
//		set_motor(1, pidDir, (u32) u);
	  }
	  timer_count = 0;
	}
  }
}
