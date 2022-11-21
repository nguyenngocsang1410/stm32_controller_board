/*
 * user_func.h
 *
 *  Created on: Sep 5, 2022
 *      Author: SangNguyen
 */

#ifndef INC_USER_FUNC_H_
#define INC_USER_FUNC_H_

#include "main.h"
#include "BUTTON.h"
/*
 #define ECD1A (ECD1A_GPIO_Port, ECD1A_Pin)
 #define ECD1B (ECD1B_GPIO_Port, ECD1B_Pin)
 #define ECD2A (ECD2A_GPIO_Port, ECD2A_Pin)
 #define ECD2B (ECD2B_GPIO_Port, ECD2B_Pin)

 #define LED1 (LED1_GPIO_Port, LED1_Pin)
 #define LED2 (LED2_GPIO_Port, LED2_Pin)
 #define PWM_1 (PWM_1_GPIO_Port, PWM_1_Pin)
 #define PWM_2 (PWM_2_GPIO_Port, PWM_2_Pin)
 #define BTN_START (BTN_START_GPIO_Port, BTN_START_Pin)
 #define BTN_STOP (BTN_STOP_GPIO_Port, BTN_STOP_Pin)
 #define MODE_PIN (MODE_PIN_GPIO_Port, MODE_PIN_Pin)
 #define M1_L (M1_L_GPIO_Port, M1_L_Pin)
 #define M1_R (M1_R_GPIO_Port, M1_R_Pin)
 #define M2_L (M2_L_GPIO_Port, M2_L_Pin)
 #define M2_R (M2_R_GPIO_Port, M2_R_Pin)
 #define LVL_PIN1 (LVL_PIN1_GPIO_Port, LVL_PIN1_Pin)
 #define LVL_PIN2 (LVL_PIN2_GPIO_Port, LVL_PIN2_Pin)
 #define LMS1 (LMS1_GPIO_Port, LMS1_Pin)
 #define LMS2 (LMS2_GPIO_Port, LMS2_Pin)
 */

typedef struct {
  GPIO_TypeDef *Port;
  uint16_t Pin;
} LedOut;

extern LedOut LED_Y, LED_G;

extern uint16_t AD_RES;
extern Button BTN_START, BTN_STOP, MODE, LVL1, LVL2;

uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax);

void active_idle();
void active_run();
void apply_state();

bool check_setting();

void stop_motor();
void set_motor(uint8_t id, int8_t dir, uint16_t pwm);
void set_motor2_dir(int8_t dir);
void flash_LED(LedOut led);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif /* INC_USER_FUNC_H_ */
