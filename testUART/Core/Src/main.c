/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c (UART)
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static char MSG[52] = { '\0' };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
uint32_t MAP(uint32_t au32_IN, uint32_t au32_INmin, uint32_t au32_INmax,
	uint32_t au32_OUTmin, uint32_t au32_OUTmax) {
  uint32_t val = au32_IN - au32_INmin;
  val = val * (au32_OUTmax - au32_OUTmin);
  val = val / (1.0 * (au32_INmax - au32_INmin));
  val = val + au32_OUTmin;
  return val;
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
	if (dir == -1) {
	  HAL_GPIO_WritePin(M2_L_GPIO_Port, M2_L_Pin, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(M2_R_GPIO_Port, M2_R_Pin, GPIO_PIN_RESET);
	  TIM4->CCR2 = pwm;
	} else if (dir == 1) {
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

volatile u32 tick_count = 0; // 1 ms/tick
volatile u8 deltaT = 20; //ms 50Hz

volatile s32 now_E1 = 0, now_E2 = 0, delta_E1 = 0, delta_E2 = 0;
volatile s32 pre_E1 = 0, pre_E2 = 0;

#define M1_PulsePerRev 1600	// tick/motor rev
#define M1_GearRatio (30.0/16.0)	// motor rev / output rev

#define M2_PulsePerRev 44	// tick/motor rev
#define M2_GearRatio 168		// motor rev / output rev
#define M2_visme_pitch	8	// translation mm/ rev

volatile float M1_velo = 0, M2_velo = 0;
volatile float M1_velo_filt = 0, M2_velo_filt = 0;
volatile float M1_preVelo = 0, M2_preVelo = 0;

volatile float M1_vref = 0, M2_vref = 0;
volatile float t = 0;
bool FLAG_runMotor = false;

volatile u8 M1_dir = 0, M2_dir = 0;
volatile u32 tick_20ms = 0;

volatile bool FLAG_sendData = false;
volatile float M1_travel = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
	tick_count = tick_count + 1;
	if (tick_count >= deltaT) {
	  //20 ms
	  tick_count = 0;
	  if (FLAG_runMotor) {
		M1_vref = 1000 * sin((2 * 3.14 / 20.0) * t);
		t = t + deltaT / 1000.0;

		now_E1 = TIM1->CNT;
		now_E2 = TIM2->CNT;

		delta_E1 = now_E1 - pre_E1;
		delta_E2 = now_E2 - pre_E2;

		if (delta_E1 < -3000) // now = 5, pre = 65530
		  delta_E1 = delta_E1 + 0xFFFF;
		else if (delta_E1 > 3000) // now = 65535, pre = 5
		  delta_E1 = delta_E1 - 0xFFFF;

		if (delta_E2 < -3000) // now = 5, pre = 65530
		  delta_E2 = delta_E2 + 0xFFFF;
		else if (delta_E2 > 3000) // now = 65535, pre = 5
		  delta_E2 = delta_E2 - 0xFFFF;

		pre_E1 = now_E1;
		pre_E2 = now_E2;

		// (ticks/s*60)/(ticks/mRev*mRev/outRev) = outRev/min
		M1_velo = (delta_E1 * 20 * 60.0)
			/ (1.0 * M1_PulsePerRev * M1_GearRatio);
		M2_velo = (delta_E2 * 20 * 60.0)
			/ (1.0 * M2_PulsePerRev * M2_GearRatio);

		M1_travel = M1_travel
			+ delta_E1 * 1.0 / (1.0 * M1_PulsePerRev * M1_GearRatio);

		// pulse/(pulse/rev*rev/Outrev) = OutRev *pitch = mm
		M1_velo_filt = -0.222 * M1_velo_filt + 0.611 * M1_velo
			+ 0.611 * M1_preVelo;
		M1_preVelo = M1_velo;

		M2_velo_filt = -0.222 * M2_velo_filt + 0.611 * M2_velo
			+ 0.611 * M2_preVelo;
		M2_preVelo = M2_velo;

//		M1_velo_e = M1_vref - M1_velo_filt;
//		M1_velo_eint = M1_velo_eint + M1_velo_e * deltaT / 1000.0;
//		M1_u = M1_Kp * M1_velo_e + M1_Ki * M1_velo_eint
//			+ M1_Kd * (M1_velo_e - M1_velo_preE) * 1000.0 / deltaT;
//		M1_velo_preE = M1_velo_e;

		float M1_pwm = fabsf(M1_vref);
		if (M1_pwm > 1000.0)
		  M1_pwm = 1000.0;
		else if (M1_pwm < 0)
		  M1_pwm = 0;

		if (M1_vref < 0)
		  M1_dir = 1;
		else
		  M1_dir = -1;
		set_motor(1, -1, 1000);
		set_motor(2, M1_dir, M1_pwm);
		FLAG_sendData = true;
	  } else {
		set_motor(1, 0, 0);
		set_motor(2, 0, 0);
	  }
	}
  }
}
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
//  memset(MSG, 0, sizeof(MSG));
//}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
u32 BTN_inTime = 0;
bool FLAG_BTN_inTime = false;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  HAL_TIM_Base_Start_IT(&htim4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	if (FLAG_sendData) {
	  FLAG_sendData = false;
	  sprintf(MSG, "%9.3f,%9.3f,%9.3f,%9.3f,%9.3f\n", M1_vref, M1_velo,
		  M1_velo_filt, M2_velo, M2_velo_filt);
	  HAL_UART_Transmit_IT(&huart2, (uint8_t*) MSG, strlen(MSG));
	}

	if (HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin) == GPIO_PIN_RESET) {
	  if (!FLAG_BTN_inTime)
		BTN_inTime = HAL_GetTick();
	  FLAG_BTN_inTime = true;
	} else if (HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin) == GPIO_PIN_SET) {
	  if (FLAG_BTN_inTime) {
		FLAG_BTN_inTime = false;
		if (HAL_GetTick() - BTN_inTime > 50) {
		  if (!FLAG_runMotor)
			FLAG_runMotor = true;
		  else {
			FLAG_runMotor = false;
			M1_travel = 0;
			sprintf(MSG, "new\n");
			HAL_UART_Transmit_IT(&huart2, (uint8_t*) MSG, strlen(MSG));
			t = 0;
		  }
		}
	  }
	}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 48;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 499;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 48;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, M1_EN_Pin|M2_L_Pin|M2_R_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN_Pin */
  GPIO_InitStruct.Pin = BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : M1_EN_Pin M2_L_Pin M2_R_Pin */
  GPIO_InitStruct.Pin = M1_EN_Pin|M2_L_Pin|M2_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
