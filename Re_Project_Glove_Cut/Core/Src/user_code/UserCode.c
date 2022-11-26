#include "UserCode.h"
#include "stdlib.h"

CLCD_I2C_Name LCD;
Button BTN_Start, BTN_Stop, BTN_Mode;

u8 mSpeed = 0;
u32 mSec = 0;
s32 mNumLoopCount = 30;
u16 nLoop = 0;
u32 nSec = 0;
u8 firstMin = 0, lastMin = 0, firstSec = 0, lastSec = 0;

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

u8 menu2_subCounter = 1;
bool firstMinSelected = 0, lastMinSelected = 0;
bool firstSecSelected = 0, lastSecSelected = 0;

bool FLAG_BlinkCursor = false;
bool FLAG_BlinkCell = false;

void start_up() {
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  HAL_TIM_Base_Start_IT(&htim4); //1ms

//  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);	//PWM1A
//  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //PWM1B
//  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);	//PWM2A
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);	//PWM2B
  HAL_Delay(10);

  buttonInit(&BTN_Start, BTN_Start_GPIO_Port, BTN_Start_Pin);
  buttonInit(&BTN_Stop, BTN_Stop_GPIO_Port, BTN_Stop_Pin);
  buttonInit(&BTN_Mode, BTN_Mode_GPIO_Port, BTN_Mode_Pin);

  // Splash screen
  LCD_Begin(&LCD, &hi2c2, 0x4E, 20, 4);
  /*******************
   ----------------------
   |   SAFETY GLOVE &   |
   |     SHOE UPPER     |
   |   CUTTING TESTER   |
   |                    |
   ----------------------
   **************************/
  LCD_Clear(&LCD);
  LED_ON();
  LCD_Print_String_At(&LCD, 1, 4, "Safety Glove &");
  LCD_Print_String_At(&LCD, 2, 6, "Shoe Upper");
  LCD_Print_String_At(&LCD, 3, 4, "Cutting Tester");

  // Restore value
  menu1_value = (u8) FLASH_ReadData(FLASH_USER_START_ADDR); // Speed
  menu2_value = (u32) FLASH_ReadData(FLASH_USER_START_ADDR + 4); // sec
  menu3_value = (s32) FLASH_ReadData(FLASH_USER_START_ADDR + 8); //set count

  // speed
  if (menu1_value > maxMenu1Value)
	menu1_value = maxMenu1Value;
  else if (menu1_value < minMenu1Value)
	menu1_value = minMenu1Value;

  // count
  if (menu2_value > maxMenu2Value)
	menu2_value = maxMenu2Value;
  else if (menu2_value < minMenu2Value)
	menu2_value = minMenu2Value;

  // time (second)
  if (menu3_value > maxMenu3Value)
	menu3_value = maxMenu3Value;
  else if (menu3_value < minMenu3Value)
	menu3_value = minMenu3Value;

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
	   |Speed:     60 rpm   |
	   |Time:   mm:ss/mm:ss |
	   |Set count: xxxx revs|
	   |Count:    0 revs    |
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
	   |>Speed:     60 rpm  |
	   | Time:      mm:ss   |
	   | Set count: xxxx    |
	   |                    |
	   ----------------------
	   **************************/
	{
	  if (refreshLCD == true) //If we are allowed to update the LCD ...
	  {
		printLCD();
		updateLCD(); // ... we update the LCD ...

		//... also, if one of the menus are already selected...
		if (menu2_selected == true) {
		  if (firstMinSelected == true || lastMinSelected == true
			  || firstSecSelected == true || lastSecSelected == true) {
			blinkMenu2();
		  } else
			updateMenu2CursorPosition();
		} else if (menu1_selected == true || menu3_selected == true
			|| menu4_selected == true) {
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
	   |Speed:     60 rpm   |
	   |Time:   mm:ss/mm:ss |
	   |Set count: xxxx revs|
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
		set_motor(2, 0, 0);
		LED_OFF();
	  }

	  //out to motor
	  set_motor(2, 1, pwm);

//	  s32 mSpeedPwm = map(mSpeed,minSpeed,maxSpeed,0,1000);
	  break;
	}
	case 4: //Result view
	  /*******************
	   ----------------------
	   |Speed:     60 rpm   |
	   |Time:   mm:ss/mm:ss |
	   |Set count: xxxx revs|
	   |Count:    0 revs    |
	   ----------------------
	   **************************/
	{
	  FLAG_run = false;
	  set_motor(1, 0, 0);
	  LED_TOGGLE();
	  printDefaultLCD();

	  HAL_Delay(300);
	  break;
	}
	default:
	  return;
  } //Switch lvl1
}

#define BTN_MODE_CLICK HAL_GPIO_ReadPin(BTN_Mode_GPIO_Port, BTN_Mode_Pin) == GPIO_PIN_RESET
#define BTN_START_CLICK HAL_GPIO_ReadPin(BTN_Start_GPIO_Port, BTN_Start_Pin) == GPIO_PIN_RESET
#define BTN_STOP_CLICK HAL_GPIO_ReadPin(BTN_Stop_GPIO_Port, BTN_Stop_Pin) == GPIO_PIN_RESET

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  switch (GPIO_Pin) {
	case BTN_Mode_Pin: {
	  if (BTN_MODE_CLICK) {
		BTN_Mode.StartPress = HAL_GetTick(); //ms
	  } else {
		BTN_Mode.StopPress = HAL_GetTick();
		BTN_Mode.timePress = BTN_Mode.StopPress - BTN_Mode.StartPress;

		if (isShortClick(&BTN_Mode) == true) {
		  if (state == 1 || state == 2) {
			state = state + 1;
			if (state == 3) {
			  switch (menuCounter) {
				case 1:
				  menu1_selected = !menu1_selected; //we change the status of the variable to the opposite
				  break;

				case 2:
				  if (menu2_selected == true) {
					switch (menu2_subCounter) {
					  case 1:	// First min digits
						firstMinSelected = 1;
						break;
					  case 2: // Last min digit
						break;
						lastMinSelected = 1;
					  case 3: // First second digit
						firstSecSelected = 1;
						break;
					  case 4: // Last second digit
						lastSecSelected = 1;
						break;
					}
					FLAG_BlinkCursor = false;
					FLAG_BlinkCell = true;

					state = 5;
					CLCD_I2C_CursorOff(&LCD);
				  } else {
					menu2_selected = true;
					FLAG_BlinkCursor = true;
				  }
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
		  } else if (state == 5) {
			state = 2;
			FLAG_BlinkCell = false;
			FLAG_BlinkCursor = true;
		  }
		} else {
		  // Long click
		  if (state == 2) {
			if (menu2_selected == true) {
			  menu2_selected = false;
			  FLAG_BlinkCell = false;
			  FLAG_BlinkCursor = false;
			} else if (state == 5) {
			  FLAG_BlinkCell = false;
			  FLAG_BlinkCursor = true;
			  state = 2;
			} else {
			  save_menu_value();
			  state = 1;
			}
		  }
		}
	  }
	  break;
	}
	case BTN_Start_Pin: {
	  if (BTN_START_CLICK) {
		BTN_Start.StartPress = HAL_GetTick();
	  } else {
		BTN_Start.StopPress = HAL_GetTick();
		BTN_Start.timePress = BTN_Start.StopPress - BTN_Start.StartPress;

		if (state == 1) {
		  state = 3;
		  reset_state();

		  if (mSec == 1)
			set_motor(1, 1, pwm);
		  else
			set_motor(1, -1, pwm);
		  //		vref = mSpeed;
		  //		FLAG_run = true;
		} else if (state == 2) {
		  state = 1;
		  save_menu_value();
		}
	  }

	  break;
	}
	case BTN_Stop_Pin: {
	  if (BTN_MODE_CLICK) {
		BTN_Mode.StartPress = HAL_GetTick(); //ms
	  } else {
		BTN_Mode.StopPress = HAL_GetTick();
		BTN_Mode.timePress = BTN_Mode.StopPress - BTN_Mode.StartPress;
		if (BTN_STOP_CLICK) {
		  BTN_Stop.StartPress = HAL_GetTick();
		} else {
		  BTN_Stop.StopPress = HAL_GetTick();
		  BTN_Stop.timePress = BTN_Stop.StopPress - BTN_Stop.StartPress;

		  if (state == 3) {
			set_motor(2, 0, 0);
			state = 4;
		  } else if (state == 4) {
			nLoop = 0;
			state = 1;
		  }
		}
	  }

	  break;
	}
	default:
	  break;
  } // End EXTI
}

u32 preEcd = 0;
void check_ecd() {
  if (state == 2) {
	if (preEcd != TIM2_count) {
	  s32 delta = preEcd - TIM2_count;
	  if (abs(delta) > 10000)
		preEcd = TIM2_count;
	  else if (abs(delta) > ecdFilter) {
		if (menu1_selected == true) {
		  if (delta > 0)
			menu1_value++;
		  else if (delta < 0)
			menu1_value--;

		  if (menu1_value > maxMenu1Value) //we do not go above 100
			menu1_value = maxMenu1Value;
		  if (menu1_value < minMenu1Value)
			menu1_value = minMenu1Value;
		} else if (menu2_selected == true) {
		  if (firstMinSelected) {
			if (delta > 0)
			  firstMin++;
			else if (delta < 0)
			  if (firstMin > 0)
				firstMin--;

			if (firstMin > 12) {
			  firstMin = 12;
			}
		  } else if (lastMinSelected) {
			if (delta > 0)
			  lastMin++;
			else if (delta < 0)
			  if (lastMin > 0)
				lastMin--;

			if (lastMin > 9) {
			  lastMin = 9;
			}
		  } else if (firstSecSelected) {
			if (delta > 0)
			  firstSec++;
			else if (delta < 0)
			  if (firstSec > 0)
				firstSec--;

			if (firstSec > 5) {
			  firstSec = 5;
			}
		  } else if (lastSecSelected) {
			if (delta > 0)
			  lastSec++;
			else if (delta < 0)
			  if (lastSec > 0)
				lastSec--;

			if (lastSec > 4) {
			  lastSec = 4;
			}
		  } else {
			if (delta > 0)
			  menu2_subCounter++;
			else if (delta < 0)
			  menu2_subCounter--;

			if (menu2_subCounter > 4) {
			  menu2_subCounter = 4;
			} else if (menu2_subCounter < 1) {
			  menu2_subCounter = 1;
			}
		  }

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
		  if (delta > 0) {
			menuCounter++;
		  } else if (delta < 0) {
			if (menuCounter > 0)
			  menuCounter--;
		  }

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
		pidDir = -mSec;
		u = fabs(u);
	  }
	  if (u > 1000)
		u = 1000;

	  if (FLAG_run == true) {
//		set_motor(1, pidDir, (u32) u);
	  }
	  timer_count = 0;
	}
  } // End TIM Elapsed
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

void printDefaultLCD() {
  /*******************
   ----------------------
   |Speed:     60 rpm   |
   |Time:   mm:ss/mm:ss |
   |Set count: xxxx revs|
   |Count:    0 revs    |
   ----------------------
   **************************/
  LCD_Print_String_At(&LCD, 1, 1, "Speed: ");
  //----------------------
  LCD_Print_String_At(&LCD, 2, 1, "Time: ");
  LCD_Print_String_At(&LCD, 3, 1, "Set count: ");
  LCD_Print_String_At(&LCD, 4, 1, "Count: ");

  //Update value
  char holder[10];

  sprintf(holder, " ");
  LCD_Print_String_At(&LCD, 1, 12, "      ");
  sprintf(holder, "%2d rpm", menu1_value);
  LCD_Print_String_At(&LCD, 1, 12, holder);

  sprintf(holder, " ");
  LCD_Print_String_At(&LCD, 2, 8, "             ");
  LCD_Set_Cursor(&LCD, 2, 8);
  LCD_Print_Clock(0);

  sprintf(holder, " ");
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

void LCD_Print_Clock(u32 sec) {
  /*
   *    |Time:   mm:ss/mm:ss |
   * */

  char holder[14];
  u8 setMin, setSec, curMin, curSec;
  curMin = sec / 60;
  curSec = sec % 60;
  setMin = mSec / 60;
  setSec = mSec % 60;
  if (curMin < 100) {
	if (setMin >= 100) {
	  sprintf(holder, "%02d:%02d/%03d:%02d", curMin, curSec, setMin, setSec);
	} else {
	  sprintf(holder, "%02d:%02d/%02d:%02d", curMin, curSec, setMin, setSec);
	}
	LCD_Print_String_At(&LCD, 2, 9, holder);
  } else {
	if (setMin >= 100) {
	  sprintf(holder, "%3d:%02d/%03d:%02d", curMin, curSec, setMin, setSec);
	} else {
	  sprintf(holder, "%3d:%02d/%02d:%02d", curMin, curSec, setMin, setSec);
	}
	LCD_Print_String_At(&LCD, 2, 8, holder);
  }
}

void printLCD() {
  /*******************
   ----------------------
   |>Speed:     60 rpm  |
   | Time:      mm:ss   |
   | Set count: xxxx    |
   |                    |
   ----------------------
   **************************/

  //These are the values which are not changing the operation
  LCD_Print_String_At(&LCD, 1, 2, "Speed: ");
  //----------------------
  LCD_Print_String_At(&LCD, 2, 2, "Time: ");
  LCD_Print_String_At(&LCD, 3, 2, "Set count: ");
  //----------------------
}

void updateLCD() {
  /*******************
   ----------------------
   |>Speed:     60 rpm  |
   | Time:      mm:ss   |
   | Set count: xxxx    |
   |                    |
   ----------------------
   **************************/
//Update value
  char holder[10];

  LCD_Print_String_At(&LCD, 1, 13, "      ");
  sprintf(holder, "%2d rpm", (int) menu1_value);
  LCD_Print_String_At(&LCD, 1, 9, holder);

  LCD_Print_String_At(&LCD, 2, 13, "      ");
  u8 setSec, setMin;
  setSec = mSec % 60;
  setMin = mSec / 60;
  if (setMin > 99) {
	sprintf(holder, "%3d:%02d", setMin, setSec);
	LCD_Print_String_At(&LCD, 2, 13, holder);
  } else {
	sprintf(holder, "%02d:%02d", setMin, setSec);
	LCD_Print_String_At(&LCD, 2, 14, holder);
  }

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
/*
 *    | Time:      mm:ss   |
 *
 * */
void updateMenu2CursorPosition() {
  switch (menu2_subCounter) {
	case 1:
	  LCD_Set_Cursor(&LCD, 2, 14);
	  break;
	case 2:
	  LCD_Set_Cursor(&LCD, 2, 15);
	  break;
	case 3:
	  LCD_Set_Cursor(&LCD, 2, 17);
	  break;
	case 4:
	  LCD_Set_Cursor(&LCD, 2, 18);
	  break;
  }
  CLCD_I2C_CursorOn(&LCD);
}

u32 lastBlink = 0;
u32 blinkInterval = 200; //ms
bool blinkOn = false;
void blinkMenu2() {
  char holder[4];
  u32 now = HAL_GetTick();
  if (now - lastBlink > blinkInterval) {
	if (blinkOn == true) {
	  {
		if (firstMinSelected) {
		  sprintf(holder, "%2d", firstMin);
		  LCD_Print_String_At(&LCD, 2, 12, holder);
		} else if (lastMinSelected) {
		  sprintf(holder, "%1d", lastMin);
		  LCD_Print_String_At(&LCD, 2, 14, holder);
		} else if (firstSecSelected) {
		  sprintf(holder, "%1d", firstSec);
		  LCD_Print_String_At(&LCD, 2, 16, holder);
		} else if (lastSecSelected) {
		  sprintf(holder, "%1d", lastSec);
		  LCD_Print_String_At(&LCD, 2, 17, holder);
		}
	  }
	  blinkOn = false;
	} else if (blinkOn == false) {
	  {
		/*
		 * mmm:ss
		 * */
		if (firstMinSelected) {
		  LCD_Print_String_At(&LCD, 2, 12, "  ");
		} else if (lastMinSelected) {
		  LCD_Print_String_At(&LCD, 2, 14, " ");
		} else if (firstSecSelected) {
		  LCD_Print_String_At(&LCD, 2, 16, "  ");
		} else if (lastSecSelected) {
		  LCD_Print_String_At(&LCD, 2, 17, "  ");
		}
	  }
	  blinkOn = true;
	}
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

void set_motor(u8 id, s8 dir, u16 val) {
  u16 pwm;
  switch (id) {
	case 1:	// BTS7960
//	  pwm = map(val, 0, 1000, 0, 499);
//	  if (dir == 1) {
//		HAL_GPIO_WritePin(M1_L_GPIO_Port, M1_L_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(M1_R_GPIO_Port, M1_R_Pin, GPIO_PIN_SET);
//		M1A_Channel = pwm;
//		M1B_Channel = 0;
//	  } else if (dir == -1) {
//		HAL_GPIO_WritePin(M1_L_GPIO_Port, M1_L_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(M1_R_GPIO_Port, M1_R_Pin, GPIO_PIN_SET);
//		M1B_Channel = pwm;
//		M1A_Channel = 0;
//	  } else if (dir == 0) //Freely run
//		  {
//		HAL_GPIO_WritePin(M1_L_GPIO_Port, M1_L_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(M1_R_GPIO_Port, M1_R_Pin, GPIO_PIN_RESET);
//		M1A_Channel = 0;
//		M1B_Channel = 0;
//	  } else	// BRAKE
//	  {
//		HAL_GPIO_WritePin(M1_L_GPIO_Port, M1_L_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(M1_R_GPIO_Port, M1_R_Pin, GPIO_PIN_SET);
//		M1B_Channel = 0;
//		M1A_Channel = 0;F
//	  }
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

void save_menu_value() {
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

void LED_Flash() {
  static u32 last_flash = 0;
  u32 now = HAL_GetTick();
  if (now - last_flash > 300) {
	LED_TOGGLE();
	last_flash = now;
  }
}
