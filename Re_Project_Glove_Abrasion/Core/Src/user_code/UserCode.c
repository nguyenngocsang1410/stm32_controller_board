#include "UserCode.h"
#include "stdlib.h"

CLCD_I2C_Name LCD;
Button BTN_Start, BTN_Stop, BTN_Mode;

s8 dir = 1;
u32 n_loop_count;
u16 n_loop;

u8 state = 1;
u8 preState = 1;

void start_up() {
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  HAL_TIM_Base_Start_IT(&htim4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);	//PWM1
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //PWM1
//  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);	//PWM2
//  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);	//PWM2

  buttonInit(&BTN_Start, BTN_Start_GPIO_Port, BTN_Start_Pin);
  buttonInit(&BTN_Stop, BTN_Stop_GPIO_Port, BTN_Stop_Pin);
  buttonInit(&BTN_Mode, BTN_Mode_GPIO_Port, BTN_Mode_Pin);

  LCD_Begin(&LCD, &hi2c2, 0x4E, 20, 4);
  /*******************
   ----------------------
   |   Taber Abrasion   |
   |   Resistance Test  |
   |   Machine          |
   |                    |
   ----------------------
   **************************/
  LCD_Clear(&LCD);
  LED_ON();
  LCD_Print_String_At(&LCD, 1, 4, "Taber Abrasion");
  LCD_Print_String_At(&LCD, 2, 4, "Resistance Test");
  LCD_Print_String_At(&LCD, 3, 4, "Machine");

  LED_ON();
  TODO("Recover Data");
  //SPEED = FLASH_ReadByte(0x4000);
  n_loop = FLASH_ReadData(FLASH_USER_START_ADDR);
  dir = (bool) FLASH_ReadData(FLASH_USER_START_ADDR + 4);

  HAL_Delay(1000);
  LCD_Clear(&LCD);
  LED_OFF();
}

void main_loop() {
  //Switch occur
  if (pre_state != state) {
	LCD_Clear(&LCD);
	pre_state = state;
	pre_TIM2 = curr_TIM2; //ECD count
  }

  switch (state) {
  case 1: //Idle
	/*******************
	 ----------------------
	 |Speed: xxrpm        |
	 |Time: mm:ss         |
	 |Count:    0 revs    |
	 |Set count: xxxx     |
	 ----------------------
	 **************************/
  {
	LCD_Print_String_At(&LCD, 1, 1, "Speed: 60rpm");
	LCD_Print_String_At(&LCD, 2, 1, "Direction: ");
	if (dir == 1) {
	  LCD_Print_String(&LCD, "CW");
	} else {
	  LCD_Print_String(&LCD, "CCW");
	}
	LCD_Print_String_At(&LCD, 3, 1, "Count:    0 revs");
	LCD_Print_String_At(&LCD, 4, 1, "Set count: ");
	LCD_Print_Int(&LCD, n_loop);
	LCD_Print_String(&LCD, " revs");
	HAL_Delay(10);
	break;
  } //case 1
  case 2: //Choose para --> blink
  {
	/*******************
	 ----------------------
	 |Speed: xxrpm        |
	 |Time: mm:ss         |
	 |Count:    0 revs    |
	 |Set count: xxxx revs|
	 ----------------------
	 **************************/
	LCD_Print_At(1, 1, "Speed: 60rpm");
	LCD_Print_At(2, 1, "Direction: ");
	if (dir == 1) {
	  LCD_Print_String("CW");
	} else {
	  LCD_Print_String("CCW");
	}
	LCD_Set_Cursor(3, 1);
	LCD_Print_String("Count:    0 revs");
	LCD_Set_Cursor(4, 1);
	LCD_Print_String("Set count: ");
	LCD_Print_Int_At(4, 12, n_loop);
	LCD_Print_String(" revs");

	curr_TIM2 = TIM2_count;
	if (pre_TIM2 != curr_TIM2) {
	  delta += curr_TIM2 - pre_TIM2;
	}
	if (delta > 100 || delta < -100) {
	  delta = 0;
	}
	if (delta >= 1) {
	  inc = 1;
	  delta = 0;
	  pre_TIM2 = curr_TIM2;
	} else if (delta <= -1) {
	  inc = -1;
	  delta = 0;
	  pre_TIM2 = curr_TIM2;
	}
	if (inc != 0) {
	  para_num += inc;
	  inc = 0;
	  if (para_num > 2)
		para_num = 2;
	  else if (para_num < 1)
		para_num = 1;
	}

	switch (para_num) {
	case 1: {
	  LCD_Print_At(2, 12, "   ");
	  HAL_Delay(LCD_dly_short);
	  LCD_Set_Cursor(2, 12);
	  if (dir == 1) {
		LCD_Print_String("CW");
	  } else {
		LCD_Print_String("CCW");
	  }
	  HAL_Delay(LCD_dly);
	  break;
	}
	case 2: {
	  LCD_Print_At(4, 12, "    ");
	  HAL_Delay(LCD_dly_short);
	  LCD_Print_Int_At(4, 12, n_loop);
	  LCD_Print_String(" revs");
	  HAL_Delay(LCD_dly);
	  break;
	}
	}
	break;
  }
  case 3: {
	curr_TIM2 = TIM2_count;
	if (pre_TIM2 != curr_TIM2) {
	  delta += curr_TIM2 - pre_TIM2;
	}
	if (delta > 100 || delta < -100) {
	  delta = 0;
	}
	if (delta >= 1) {
	  inc = 1;
	  delta = 0;
	  pre_TIM2 = curr_TIM2;
	} else if (delta <= -1) {
	  inc = -1;
	  delta = 0;
	  pre_TIM2 = curr_TIM2;
	}

	switch (para_num) {
	case 1: {
	  if (inc != 0) {
		dir += inc;
		if (dir > 1)
		  dir = 1;
		else if (dir < 1)
		  dir = 0;
		inc = 0;
	  }
	  LCD_Print_At(2, 1, "Direction: ");
	  LCD_Print_At(2, 12, "   ");
	  HAL_Delay(LCD_dly_short);
	  LCD_Set_Cursor(2, 12);
	  if (dir == 1) {
		LCD_Print_String("CW");
	  } else {
		LCD_Print_String("CCW");
	  }

	  HAL_Delay(LCD_dly);
	  break;
	}
	case 2: {
	  if (inc != 0) {
		n_loop += inc * 10;
		inc = 0;
		if (n_loop > 9990)
		  n_loop = 10;
		else if (n_loop < 10)
		  n_loop = 9990;
	  }
	  LCD_Set_Cursor(4, 1);
	  LCD_Print_String("Set count: ");
	  LCD_Print_At(4, 12, "    ");
	  if (n_loop < 10)
		LCD_Set_Cursor(4, 15);
	  else if (n_loop < 100)
		LCD_Set_Cursor(4, 14);
	  else if (n_loop < 1000)
		LCD_Set_Cursor(4, 13);
	  else if (n_loop >= 1000)
		LCD_Set_Cursor(4, 12);
	  HAL_Delay(LCD_dly_short);
	  LCD_Print_Int(n_loop);
	  LCD_Print_At(4, 16, " revs");
	  HAL_Delay(LCD_dly);
	  break;
	}
	}
	break;
  } //case 3
  case 5: //Run
	/*******************
	 ----------------------
	 |Speed: xxrpm        |
	 |Time: mm:ss         |
	 |Count: xxxx round   |
	 |Set round: xxxx     |
	 ----------------------
	 **************************/
  {
	LCD_Print_At(1, 1, "Speed: 60rpm");
	/*
	 LCD_Print_At(2,1,"Time: ");    //Time mm:ss/MM:SS
	 t = LCDclock(FALSE);
	 run_MIN = (t/60);
	 run_SEC = t%60;

	 if (run_MIN < 10) LCD_Print_Char('0');
	 LCD_Print_Int(run_MIN);
	 LCD_Print_Char(':');
	 if (run_SEC < 10) LCD_Print_Char('0');
	 LCD_Print_Int(run_SEC);
	 */

	LCD_Print_At(2, 1, "Direction: ");
	if (dir == 1) {
	  LCD_Print_String("CW");
	} else {
	  LCD_Print_String("CCW");
	}

	LCD_Set_Cursor(3, 1);
	LCD_Print_String("Count: ");

	s32 motor_posi = TIM1_count;
	if (motor_posi < 0)
	  n_loop_count = -motor_posi / (990);
	else
	  n_loop_count = motor_posi / (990);
	if (n_loop_count < 10)
	  LCD_Set_Cursor(3, 11);
	else if (n_loop_count < 100)
	  LCD_Set_Cursor(3, 10);
	else if (n_loop_count < 1000)
	  LCD_Set_Cursor(3, 9);
	else
	  LCD_Set_Cursor(3, 8);
	LCD_Print_Int(n_loop_count);
	LCD_Print_String(" revs");

	LCD_Set_Cursor(4, 1);
	LCD_Print_String("Set count: ");
	LCD_Print_Int(n_loop);
	LCD_Print_String(" revs");

	if (n_loop_count == n_loop) {
	  state = 6;
	  set_motor(0, 0);
	  GPIO_WriteLow(LED_PANEL);
	}

	//pid controller
	//Input = % PWM --> Output: rpm
	curr_pid = millis();
	u32 deltaT_ms = curr_pid - pre_pid;
	if (deltaT_ms >= pid_interval) {
	  pre_pid = curr_pid;
	  u32 motor_avelo = speed_m; //ticks/20ms
	  motor_avelo = motor_avelo * 50 * 60; //ticks/m

	  error = SPEED * 990 - motor_avelo; //ticks/m
	  eint = eint + error * deltaT_ms; //rpmm

	  s32 u = kp * error * 60000 + ki * eint;   //%PWM
	  u = u / 60000;
	}
	//out to motor
	if (dir == 1) {
	  set_motor(1, SPEED);
	} else {
	  set_motor(-1, SPEED);
	}
	break;
  }   //case 5
  case 6: //Result
	/*******************
	 ------------------
	 |Speed: xxrpm        |
	 |Time: mm:ss         |
	 |Count: xxxx round   |
	 |Set round: xxxx     |
	 ------------------
	 **************************/
  {
	set_motor(0, 0);
	LCD_Print_At(1, 1, "Speed: 60rpm");

	/*
	 LCD_Print_At(2,1,"Time: ");    //Time mm:ss/MM:SS
	 if (run_MIN < 10) LCD_Print_Char('0');
	 LCD_Print_Int(run_MIN);
	 LCD_Print_Char(':');
	 if (run_SEC < 10) LCD_Print_Char('0');
	 LCD_Print_Int(run_SEC);
	 */

	LCD_Print_At(2, 1, "Direction: ");
	if (dir == 1) {
	  LCD_Print_String("CW");
	} else {
	  LCD_Print_String("CCW");
	}
	LCD_Set_Cursor(3, 1);
	LCD_Print_String("Count:      revs");
	if (n_loop_count < 10)
	  LCD_Set_Cursor(3, 11);
	else if (n_loop_count < 100)
	  LCD_Set_Cursor(3, 10);
	else if (n_loop_count < 1000)
	  LCD_Set_Cursor(3, 9);
	else
	  LCD_Set_Cursor(3, 8);
	LCD_Print_Int(n_loop_count);

	LCD_Set_Cursor(4, 1);
	LCD_Print_String("Set count: ");
	LCD_Print_Int(n_loop);
	LCD_Print_String(" revs");

	HAL_Delay(LCD_dly);

	LCD_Clear();
	HAL_Delay(LCD_dly_short);
	break;
  } //case 6
  } //Switch lvl1
}
void LCD_Begin(CLCD_I2C_Name *LCD, I2C_HandleTypeDef *hi2c_CLCD,
	uint8_t Address, uint8_t Colums, uint8_t Rows) {
  CLCD_I2C_Init(LCD, hi2c_CLCD, Address, Colums, Rows);
}
void LCD_Set_Cursor(CLCD_I2C_Name *LCD, u8 a, u8 b) {
  a -= 1;
  b -= 1;
  CLCD_I2C_SetCursor(LCD, b, a);
}
void LCD_Print_Char(CLCD_I2C_Name *LCD, char data) {
  CLCD_I2C_WriteChar(LCD, data);
}
void LCD_Clear(CLCD_I2C_Name *LCD) {
  CLCD_I2C_Clear(LCD);
  CLCD_I2C_ReturnHome(LCD);
}
void LCD_Print_String(CLCD_I2C_Name *LCD, char *data) {
  CLCD_I2C_WriteString(LCD, data);
}
void LCD_Print_Int(CLCD_I2C_Name *LCD, s32 data) {
  char temp_buffer[20];
  itoa(data, temp_buffer, 10);
  u8 i;
  for (i = 0; temp_buffer[i] != '\0'; i++)
	LCD_Print_Char(LCD, temp_buffer[i]); //Split the string using pointers and call the Char function
}
void LCD_Print_String_At(CLCD_I2C_Name *LCD, u8 a, u8 b, char *data) {
  LCD_Set_Cursor(LCD, a, b);
  LCD_Print_String(LCD, data);
}
void LCD_Print_Int_At(CLCD_I2C_Name *LCD, u8 a, u8 b, s32 data) {
  LCD_Set_Cursor(LCD, a, b);
  LCD_Print_Int(LCD, data);
}
void LCD_Return_Home(CLCD_I2C_Name *LCD) {
  CLCD_I2C_ReturnHome(LCD);
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  /* Mode button */
  if (GPIO_Pin == BTN_Mode_Pin) {
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
			  state = 5;
			  break;

			case 4:
			  menu4_selected = !menu4_selected;
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
			  menu1_Value);
		  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR + 4,
			  menu2_Value);
		  HAL_FLASH_Lock();
		} else if (state >= 4) {
		  STOP_cmd = true;
		  STOP_from = HAL_GetTick();
		}
	  }
	}
  }

  /* Start button */
  if (GPIO_Pin == BTN_Start_Pin) {
	if (HAL_GPIO_ReadPin(BTN_Start_GPIO_Port, BTN_Start_Pin)
		== GPIO_PIN_RESET) {
	  if (state == 1) {
		state = 3;
		reset_state();
		set_motor(1, 30);
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
			menu1_Value);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR + 4,
			menu2_Value);
		HAL_FLASH_Lock();
	  }
	} else {

	}
  }

  /* Stop button */
  if (GPIO_Pin == BTN_Stop_Pin) {
	if (HAL_GPIO_ReadPin(BTN_Mode_GPIO_Port, BTN_Stop_Pin) == GPIO_PIN_RESET) {
	  if (state == 3) {
		set_motor(0, 0);
		state = 4;
	  } else if (state == 4) {
		set_motor(0, 0);
		STOP_cmd = false;
	  }
	}
  }
}
