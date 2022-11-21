/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OSC1_Pin GPIO_PIN_0
#define OSC1_GPIO_Port GPIOD
#define OSC2_Pin GPIO_PIN_1
#define OSC2_GPIO_Port GPIOD
#define ECD2A_Pin GPIO_PIN_0
#define ECD2A_GPIO_Port GPIOA
#define ECD2B_Pin GPIO_PIN_1
#define ECD2B_GPIO_Port GPIOA
#define LED_Y_Pin GPIO_PIN_4
#define LED_Y_GPIO_Port GPIOA
#define LED_G_Pin GPIO_PIN_5
#define LED_G_GPIO_Port GPIOA
#define PWM1_F_Pin GPIO_PIN_6
#define PWM1_F_GPIO_Port GPIOA
#define PWM1_B_Pin GPIO_PIN_7
#define PWM1_B_GPIO_Port GPIOA
#define BTN_STOP_Pin GPIO_PIN_0
#define BTN_STOP_GPIO_Port GPIOB
#define BTN_START_Pin GPIO_PIN_1
#define BTN_START_GPIO_Port GPIOB
#define MODE_SW_Pin GPIO_PIN_10
#define MODE_SW_GPIO_Port GPIOB
#define LMS_B_Pin GPIO_PIN_12
#define LMS_B_GPIO_Port GPIOB
#define LMS_B_EXTI_IRQn EXTI15_10_IRQn
#define M1_EN_Pin GPIO_PIN_13
#define M1_EN_GPIO_Port GPIOB
#define M2_L_Pin GPIO_PIN_14
#define M2_L_GPIO_Port GPIOB
#define M2_R_Pin GPIO_PIN_15
#define M2_R_GPIO_Port GPIOB
#define ECD1A_Pin GPIO_PIN_8
#define ECD1A_GPIO_Port GPIOA
#define ECD1B_Pin GPIO_PIN_9
#define ECD1B_GPIO_Port GPIOA
#define LVL_SW1_Pin GPIO_PIN_3
#define LVL_SW1_GPIO_Port GPIOB
#define LVL_SW2_Pin GPIO_PIN_4
#define LVL_SW2_GPIO_Port GPIOB
#define LMS_E_Pin GPIO_PIN_5
#define LMS_E_GPIO_Port GPIOB
#define LMS_E_EXTI_IRQn EXTI9_5_IRQn
#define LMS_M_Pin GPIO_PIN_6
#define LMS_M_GPIO_Port GPIOB
#define LMS_M_EXTI_IRQn EXTI9_5_IRQn
#define PWM2_Pin GPIO_PIN_7
#define PWM2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
