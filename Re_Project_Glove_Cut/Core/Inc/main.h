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
#define ECD_2A_Pin GPIO_PIN_0
#define ECD_2A_GPIO_Port GPIOA
#define ECD_2B_Pin GPIO_PIN_1
#define ECD_2B_GPIO_Port GPIOA
#define Y10_Pin GPIO_PIN_4
#define Y10_GPIO_Port GPIOA
#define Y11_Pin GPIO_PIN_5
#define Y11_GPIO_Port GPIOA
#define PWM_1A_Pin GPIO_PIN_6
#define PWM_1A_GPIO_Port GPIOA
#define PWM_1B_Pin GPIO_PIN_7
#define PWM_1B_GPIO_Port GPIOA
#define X0_Pin GPIO_PIN_0
#define X0_GPIO_Port GPIOB
#define X1_Pin GPIO_PIN_1
#define X1_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_10
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_11
#define SDA_GPIO_Port GPIOB
#define Y3_Pin GPIO_PIN_12
#define Y3_GPIO_Port GPIOB
#define Y4_Pin GPIO_PIN_13
#define Y4_GPIO_Port GPIOB
#define Y5_Pin GPIO_PIN_14
#define Y5_GPIO_Port GPIOB
#define Y6_Pin GPIO_PIN_15
#define Y6_GPIO_Port GPIOB
#define ECD_1A_Pin GPIO_PIN_8
#define ECD_1A_GPIO_Port GPIOA
#define ECD_1B_Pin GPIO_PIN_9
#define ECD_1B_GPIO_Port GPIOA
#define Y7_Pin GPIO_PIN_10
#define Y7_GPIO_Port GPIOA
#define X2_Pin GPIO_PIN_15
#define X2_GPIO_Port GPIOA
#define X3_Pin GPIO_PIN_3
#define X3_GPIO_Port GPIOB
#define X3_EXTI_IRQn EXTI3_IRQn
#define X4_Pin GPIO_PIN_4
#define X4_GPIO_Port GPIOB
#define X4_EXTI_IRQn EXTI4_IRQn
#define X5_Pin GPIO_PIN_5
#define X5_GPIO_Port GPIOB
#define X5_EXTI_IRQn EXTI9_5_IRQn
#define X6_Pin GPIO_PIN_6
#define X6_GPIO_Port GPIOB
#define PWM_2A_Pin GPIO_PIN_7
#define PWM_2A_GPIO_Port GPIOB
#define PWM_2B_Pin GPIO_PIN_8
#define PWM_2B_GPIO_Port GPIOB
#define X7_Pin GPIO_PIN_9
#define X7_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
