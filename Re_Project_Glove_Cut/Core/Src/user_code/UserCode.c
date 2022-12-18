#include "UserCode.h"
#include "stdlib.h"

u32 PROBE_ON = 0, PROBE_OFF = 0;

CLCD_I2C_Name LCD;
Button BTN_Start, BTN_Stop, BTN_Mode;

u8 mSpeed = 0;
u32 mSec = 0;
s32 mNumLoopCount = 30;
u16 nLoop = 0, lastLoop = 0;
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
bool refreshMenu1 = false;
bool refreshMenu2 = false;
bool refreshMenu3 = false;
bool refreshClock = false;

float curSpeed = 0, preSpeed = 0;
u32 curPos = 0, prePos = 0;
u32 curTime = 0, preTime = 0;
u32 timeInterval = 20; //ms

float kp = 10, ki = 0, kd = 0;
float eint = 0, e = 0, preE = 0;
float vref = 0;
u32 pre_posi = 0;
s8 pidDir = 1;

bool FLAG_run = false;

u32 pwm = 500;

u8 menu2_subCounter = 1;
bool firstMinSelected = 0, lastMinSelected = 0;
bool firstSecSelected = 0, lastSecSelected = 0;

void setup() {
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  HAL_TIM_Base_Start_IT(&htim4); //1ms
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);    //PWM2B

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
  menu2_value = (u32) FLASH_ReadData(FLASH_USER_START_ADDR + 4); // set sec
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

void loop() {
  check_state();
  check_ecd();

  switch (state) {
    case 1:
      //Idle
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
    case 2:
      //Choose para --> blink
      /*******************
       ----------------------
       |>Speed:     60 rpm  |
       | Time:      mm:ss   |
       | Set count: xxxx    |
       |                    |
       ----------------------
       **************************/
    {

      if (menu2_selected == true) {
        if (firstMinSelected == true || lastMinSelected == true
            || firstSecSelected == true || lastSecSelected == true) {
          // Do nothing
          if (refreshMenu2) {
            update_Menu2();
            switch (menu2_subCounter) {
              case 1:
                LCD_Set_Cursor(&LCD, 2, 9);
                break;
              case 2:
                LCD_Set_Cursor(&LCD, 2, 10);
                break;
              case 3:
                LCD_Set_Cursor(&LCD, 2, 12);
                break;
              case 4:
                LCD_Set_Cursor(&LCD, 2, 13);
                break;
              default:
                break;
            }
            refreshMenu2 = false;
          }
        } else
          blinkCursor();
      } else if (menu1_selected == true) {
        if (refreshMenu1) {
          refreshMenu1 = false;
          update_Menu1();
        }
      } else if (menu3_selected == true) {
        if (refreshMenu3) {
          refreshMenu3 = false;
          update_Menu3();
        }
      }

      if (refreshLCD == true) //If we are allowed to update the LCD ...
      {
        refreshLCD = false; //reset the variable - wait for a new trigger

        LCD_Clear(&LCD);
        printMenuName();
        printMenuValue(); // ... we update the LCD ...

        //... also, if one of the menus are already selected...
        updateSelection(); //update the position
      }

      if (refreshSelection == true) //if the selection is changed
      {
        refreshSelection = false; // reset the variable - wait for a new trigger
        updateSelection(); //update the selection on the LCD
      }
      break;
    }
    case 3:
      // Run
      /*******************
       ----------------------
       |Speed:     60 rpm   |
       |Time:   mm:ss/mm:ss |
       |Set count: xxxx revs|
       |Count:    0 revs    |
       ----------------------
       **************************/
    {
      //out to motor
      set_motor(2, 1, pwm);

      if (refreshLCD) {
        printDefaultLCD();
        refreshLCD = false;
      }

      u32 motor_posi = TIM1_count;
      s32 delta = motor_posi - pre_posi;
      if (abs(delta) > totalPulse) {
        nLoop++;
        pre_posi = motor_posi;
      }

      // Stop condition
      if (nLoop >= menu3_value) {
        nLoop = menu3_value;
        stopCmd();
      } else if (nSec >= menu2_value) {
        nSec = menu2_value;
        stopCmd();
      }

      if (lastLoop != nLoop) {
        lastLoop = nLoop;

        char holder[10];
        LCD_Print_String_At(&LCD, 4, 8, "         ");
        if (nLoop < 2) {
          sprintf(holder, "%4d rev", nLoop);
        } else {
          sprintf(holder, "%4d revs", nLoop);
        }
        LCD_Print_String_At(&LCD, 4, 8, holder);
      }

      if (refreshClock) {
        refreshClock = false;
        LCD_Print_Clock(nSec);
      }

      break;
    }
    case 4:
      //Result view
      /*******************
       ----------------------
       |Speed:     60 rpm   |
       |Time:   mm:ss/mm:ss |
       |Set count: xxxx revs|
       |Count:    0 revs    |
       ----------------------
       **************************/
    {
      PROBE_ON = 0;
      if (refreshLCD) {
        printDefaultLCD();
        refreshLCD = false;

        LCD_Print_Clock(nSec);

        char holder[10];
        LCD_Print_String_At(&LCD, 4, 8, "         ");
        if (nLoop < 2) {
          sprintf(holder, "%4d rev", nLoop);
        } else {
          sprintf(holder, "%4d revs", nLoop);
        }
        LCD_Print_String_At(&LCD, 4, 8, holder);
      }
      FLAG_run = false;
      set_motor(2, 0, 0);
      LED_TOGGLE();

      HAL_Delay(500);
      break;
    }
    default:
      return;
  } //Switch lvl1
}

#define BTN_MODE_CLICK HAL_GPIO_ReadPin(BTN_Mode_GPIO_Port, BTN_Mode_Pin) == GPIO_PIN_RESET
#define BTN_START_CLICK HAL_GPIO_ReadPin(BTN_Start_GPIO_Port, BTN_Start_Pin) == GPIO_PIN_RESET
#define BTN_STOP_CLICK HAL_GPIO_ReadPin(BTN_Stop_GPIO_Port, BTN_Stop_Pin) == GPIO_PIN_RESET

#define PROBE_CLICK HAL_GPIO_ReadPin(PROBE_GPIO_Port, PROBE_Pin) == GPIO_PIN_RESET

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  switch (GPIO_Pin) {
    case BTN_Mode_Pin: {
      if (BTN_MODE_CLICK) {
        BTN_Mode.StartPress = HAL_GetTick(); //ms
      } else {
        BTN_Mode.StopPress = HAL_GetTick();
        BTN_Mode.timePress = BTN_Mode.StopPress - BTN_Mode.StartPress;

        u8 check = isShortClick(&BTN_Mode);
        if (check == 1) // MODE click
        {
          if (state == 1 || state == 2) {
            state = state + 1;
            if (state == 3) {
              switch (menuCounter) {
                case 1:
                  menu1_selected = !menu1_selected; //we change the status of the variable to the opposite
                  refreshSelection = true;
                  break;

                case 2:
                  if (menu2_selected == true) {
                    switch (menu2_subCounter) {
                      case 1:    // First min digits
                        firstMinSelected = !firstMinSelected;
                        break;
                      case 2: // Last min digit
                        lastMinSelected = !lastMinSelected;
                        break;
                      case 3: // First second digit
                        firstSecSelected = !firstSecSelected;
                        break;
                      case 4: // Last second digit
                        lastSecSelected = !lastSecSelected;
                        break;
                    }
                    if (firstMinSelected || lastMinSelected || firstSecSelected
                        || lastSecSelected) {
                      CLCD_I2C_CursorOff(&LCD);
                      CLCD_I2C_BlinkOn(&LCD);
                    } else {
                      CLCD_I2C_BlinkOff(&LCD);
                    }
                  } else {
                    menu2_selected = true;
                    refreshSelection = true;
                  }
                  break;

                case 3:
                  menu3_selected = !menu3_selected;
                  refreshSelection = true;
                  break;
                default:
                  break;
              }
              state = 2;
            } else if (state == 2) {
              firstMin = (u32) ((u32) menu2_value / 60) / 10;
              lastMin = (u32) ((u32) menu2_value / 60) % 10;
              firstSec = (u32) ((u32) menu2_value % 60) / 10;
              lastSec = (u32) ((u32) menu2_value % 60) % 10;
            }
          }
        }
          // MODE Long click
        else if (check == 2) {
          if (state == 2) {
            if (menu2_selected == true) {
              if (firstMinSelected) {
                firstMinSelected = false;
              } else if (lastMinSelected) {
                lastMinSelected = false;
              } else if (firstSecSelected) {
                firstSecSelected = false;
              } else if (lastSecSelected) {
                lastSecSelected = false;
              }

              CLCD_I2C_BlinkOff(&LCD);
              CLCD_I2C_CursorOff(&LCD);

              menu2_selected = false;
              refreshSelection = true;

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
        if (isShortClick(&BTN_Start) == 1) {
          if (state == 1) {
            state = 3;
            PROBE_ON = 0;
            reset_state();

            if (menu2_value == 1)
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
      }
      break;
    }

    case BTN_Stop_Pin: {
      if (BTN_STOP_CLICK == 1) {
        BTN_Stop.StartPress = HAL_GetTick();
      } else {
        BTN_Stop.StopPress = HAL_GetTick();
        BTN_Stop.timePress = BTN_Stop.StopPress - BTN_Stop.StartPress;
        if (isShortClick(&BTN_Stop)) {
          if (state == 3) {
            set_motor(2, 0, 0);
            state = 4;
          } else if (state == 4) {
            PROBE_ON = 0;

            nLoop = 0;
            state = 1;
            nSec = 0;

            refreshLCD = true;
            LED_OFF();
            set_motor(2, 0, 0);
          }
        }
      }
      break;
    }

    case PROBE_Pin:
      if (HAL_GPIO_ReadPin(PROBE_GPIO_Port, PROBE_Pin) == GPIO_PIN_SET) {
        if (state == 3)
          PROBE_ON = HAL_GetTick();
      } else // PROBE_CLICK
      {
        PROBE_ON = 0;
      }
      break;

    default:
      break;
  } // End EXTI
}

u32 timer_count = 0;
u32 tick1s = 0;
float u = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) // 1ms
  {
    timer_count++;

    if (timer_count >= timeInterval) // 20ms 50Hz
    {
      curPos = TIM1_count;
      s32 deltaPos = curPos > prePos ? curPos - prePos : -1 * (prePos - curPos);
      prePos = curPos;
      curSpeed = ((float) labs(deltaPos) / (totalPulse)) * 60.0 / (timeInterval / 1000.0);
      e = curSpeed - vref;
      eint = eint + e * timeInterval;
      float de = (e - preE) / timeInterval;
      preE = e;
      u = kp * e + ki * eint + kd * de;   //%PWM

      if (u < 0) {
        pidDir = -menu2_value;
        u = fabsf(u);
      }
      if (u > 1000)
        u = 1000;

      if (FLAG_run == true) {
//		set_motor(1, pidDir, (u32) u);
      }
      timer_count = 0;
    }

    tick1s++;
    if (tick1s >= 1000) {
      if (state == 3) {
        nSec++;
        refreshClock = true;
      }
      tick1s = 0;
    }
  } // End TIM Elapsed
}

int DEBOUNCE_PROBE = 100;
u32 lastProbe = 0;

void check_state() {
// If Enter new state
  if (preState != state) {
    LCD_Clear(&LCD);
    preState = state;
    refreshLCD = true;
    refreshSelection = true;
  }
  if (state == 3) {
    u32 now = HAL_GetTick();

    if (PROBE_ON != 0) {
      if ((now - PROBE_ON > DEBOUNCE_PROBE) && (now - PROBE_ON < 3000)) {
        lastProbe = PROBE_ON;
        stopCmd();
      }
    }
  }
}

u32 preEcd = 0;

void check_ecd() {
  if (state == 2) {
    if (preEcd != TIM2_count) {
      s32 delta = preEcd - TIM2_count;
      if (abs(delta) > 10000) // wrap
        preEcd = TIM2_count;
      else if (abs(delta) > ecdFilter) {
        preEcd = TIM2_count;

        // Menu 1
        if (menu1_selected == true) {
          if (delta > 0)
            menu1_value++;
          else if (delta < 0)
            menu1_value--;

          if (menu1_value > maxMenu1Value) //we do not go above 100
            menu1_value = maxMenu1Value;
          if (menu1_value < minMenu1Value)
            menu1_value = minMenu1Value;

          refreshMenu1 = true;
        }
          // Menu 2
        else if (menu2_selected == true) {

          if (firstMinSelected == true || lastMinSelected == true
              || firstSecSelected == true || lastSecSelected == true) {
            if (firstMinSelected) {
              if (delta > 0)
                firstMin++;
              else if (delta < 0)
                if (firstMin > 0)
                  firstMin--;

              if (firstMin > 9) {
                firstMin = 9;
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

              if (lastSec > 9) {
                lastSec = 9;
              }
            }

            refreshMenu2 = true;
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
            u8 _mCursorPos = 1;
            switch (menu2_subCounter) {
              case 1:
                _mCursorPos = 9;
                break;
              case 2:
                _mCursorPos = 10;
                break;
              case 3:
                _mCursorPos = 12;
                break;
              case 4:
                _mCursorPos = 13;
                break;
              default:
                break;
            }
            LCD_Set_Cursor(&LCD, 2, _mCursorPos);
          }
        }
          // Menu 3
        else if (menu3_selected == true) {
          if (delta > 0)
            menu3_value++;
          else if (delta < 0)
            menu3_value--;

          if (menu3_value > maxMenu3Value)
            menu3_value = maxMenu3Value;
          if (menu3_value < minMenu3Value)
            menu3_value = minMenu3Value; //rpm

          refreshMenu3 = true;
        } else {
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

          refreshSelection = true;
        }
      }
    }
  } else {
    preEcd = TIM2_count;
  }
  return;
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
  uint32_t data = *(__IO uint32_t *) (addr);
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

/*******************
 ----------------------
 |Speed:     60 rpm   |
 |Time:   mm:ss/mm:ss |
 |Set count: xxxx revs|
 |Count:    0 revs    |
 ----------------------
 **************************/
void update_Menu1() {
  char holder[10];
  LCD_Print_String_At(&LCD, 1, 12, "      ");
  sprintf(holder, "%2d rpm", menu1_value);
  LCD_Print_String_At(&LCD, 1, 12, holder);
}

void update_Menu2() {
  char holder[10];
// fist Min
  if (firstMinSelected) {
    LCD_Print_String_At(&LCD, 2, 8, "  ");
    sprintf(holder, "%2d", firstMin);
    LCD_Print_String_At(&LCD, 2, 8, holder);
    LCD_Set_Cursor(&LCD, 2, 8);
  }
// last Min
  else if (lastMinSelected) {
    sprintf(holder, "%1d", lastMin);
    LCD_Print_String_At(&LCD, 2, 10, holder);
    LCD_Set_Cursor(&LCD, 2, 8);
  }
// first Sec
  else if (firstSecSelected) {
    sprintf(holder, "%1d", firstSec);
    LCD_Print_String_At(&LCD, 2, 12, holder);
    LCD_Set_Cursor(&LCD, 2, 8);
  }
// last Sec
  else if (lastSecSelected) {
    sprintf(holder, "%1d", lastSec);
    LCD_Print_String_At(&LCD, 2, 13, holder);
    LCD_Set_Cursor(&LCD, 2, 8);
  } else {
    LCD_Print_String_At(&LCD, 2, 8, "  ");
    sprintf(holder, "%2d", firstMin);
    LCD_Print_String_At(&LCD, 2, 8, holder);
    LCD_Set_Cursor(&LCD, 2, 8);

    sprintf(holder, "%1d", lastMin);
    LCD_Print_String_At(&LCD, 2, 10, holder);
    LCD_Set_Cursor(&LCD, 2, 8);

    sprintf(holder, "%1d", firstSec);
    LCD_Print_String_At(&LCD, 2, 12, holder);
    LCD_Set_Cursor(&LCD, 2, 8);

    sprintf(holder, "%1d", lastSec);
    LCD_Print_String_At(&LCD, 2, 13, holder);
    LCD_Set_Cursor(&LCD, 2, 8);
  }

  LCD_Print_String_At(&LCD, 2, 11, ":");
}

void update_Menu3() {
  char holder[10];
  LCD_Print_String_At(&LCD, 3, 12, "        ");
  sprintf(holder, "%3d revs", (int) menu3_value);
  LCD_Print_String_At(&LCD, 3, 12, holder);
}

void LCD_Print_Clock(u32 sec) {
  /*
   *    |Time:   mm:ss/mm:ss |
   * */

  char holder[14];
  u8 setMin, setSec, curMin, curSec;
  curMin = sec / 60;
  curSec = sec % 60;
  setMin = menu2_value / 60;
  setSec = menu2_value % 60;
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

void printMenuName() {
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

void printMenuValue() {
  /*******************
   ----------------------
   |>Speed:     60 rpm  |
   | Time:      mm:ss   |
   | Set count: xxxx    |
   |                    |
   ----------------------
   **************************/
//Update value
  update_Menu1();
  update_Menu2();
  update_Menu3();
}

/*
 *    | Time:      mm:ss   |
 *
 * */
u32 lastBlink = 0;
bool _isBlink = false;

void blinkCursor() {
  u32 now = HAL_GetTick();
  if (now - lastBlink > 250) {
    lastBlink = now;
    if (_isBlink) {
      CLCD_I2C_CursorOn(&LCD);
      _isBlink = false;
    } else {
      CLCD_I2C_CursorOff(&LCD);
      _isBlink = true;
    }
  }
}

void updateSelection() {
//Clear display's ">" parts
  LCD_Print_String_At(&LCD, 1, 1, " ");
  LCD_Print_String_At(&LCD, 2, 1, " ");
  LCD_Print_String_At(&LCD, 3, 1, " ");
  LCD_Print_String_At(&LCD, 4, 1, " ");

//When a menu is selected ">" becomes "X"
  if (menu1_selected == true) {
    LCD_Print_String_At(&LCD, 1, 1, "*");
  } else if (menu2_selected == true) {
    LCD_Print_String_At(&LCD, 2, 1, "*");
    LCD_Set_Cursor(&LCD, 2, 9);
  } else if (menu3_selected == true) {
    LCD_Print_String_At(&LCD, 3, 1, "*");
  } else if (menu4_selected == true) {
    LCD_Print_String_At(&LCD, 4, 1, "*");
  } else {
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
}

void set_motor(u8 id, s8 dir, u16 val) {
  u16 pwm;
  switch (id) {
    case 1:    // BTS7960
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
    case 2:    // L298N
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
  nSec = 0;
}

void save_menu_value() {
  menu2_value = (firstMin * 10 + lastMin) * 60 + firstSec * 10 + lastSec;
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef EraseInit;
  EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInit.PageAddress = FLASH_USER_START_ADDR;
  EraseInit.NbPages = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR)
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

void stopCmd() {
  if (state == 3) {
    state = 4;
    set_motor(2, 0, 0);
    LED_OFF();
    refreshLCD = true;
  }
}
