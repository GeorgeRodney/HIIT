/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"
#include <stdint.h>


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM10_Init(void);

void ButtonPollingDebounced(void);
void ExecutePress(Buttons button);

TIM_HandleTypeDef htim10;
volatile uint32_t msCounter = 0;  // Millisecond counter
volatile uint32_t seconds = 0;

ButtonGPIOConfig buttonConfigs[BUTTON_COUNT] = {
  {BUTTON_PIN, BUTTON_GPIO_PORT}
};

int32_t bCounter[BUTTON_COUNT] = {0};
uint8_t bPressed[BUTTON_COUNT] = {NOT_PRESSED};

SystemState sysState_ = PAUSE;


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM10_Init();
  HAL_TIM_Base_Start_IT(&htim10);

  while (1)
  {
    ButtonPollingDebounced();
  }

}

// // Check timing
// if((msCounter - prevTime) >= 500)
// {
//   // HAL_GPIO_TogglePin(MAIN_LED_GPIO_Port,MAIN_LED_Pin);
//   prevTime = msCounter;
// }

void ButtonPollingDebounced(void)
{

  for(Buttons button = 0; button < BUTTON_COUNT; button++)
  {
    
    // Increment or decrement the button counter
    GPIO_PinState pressResult =  HAL_GPIO_ReadPin(buttonConfigs[button].port, buttonConfigs[button].pin);
    if( (pressResult == LOW) && (bCounter[button] < COUNTER_MAX))
    {
      bCounter[button]++;
    }
    else if ((pressResult == HIGH) && (bCounter[button] > 0))
    {
      bCounter[button]--;
    }

    // Check transition: NOT_PRESSED -> PRESSED?
    if((bCounter[button] > PRESSED_COUNT) && 
          (bPressed[button] == NOT_PRESSED))
    {
      bPressed[button] = PRESSED;
      ExecutePress(button);
    }

    // Check transition: PRESSED -> NOT_PRESSED?
    if((bCounter[button] < NOT_PRESSED_COUNT) &&
          (bPressed[button] == PRESSED))
    {
      bPressed[button] = NOT_PRESSED;
    }

  }

}

void ExecutePress(Buttons button)
{
  switch (button){

    case PLAY_PAUSE: // PLAY/PAUSE BUTTON
      if (sysState_ == PAUSE)
      {
        // START_WORKOUT()
        HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_Pin, GPIO_PIN_SET);
        sysState_ = PLAY;
      }
      else if (sysState_ == PLAY)
      {
        // PAUSE_MENU()
        HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_Pin, GPIO_PIN_RESET);
        sysState_ = PAUSE;
      }

      break;
    
    case TOGGLE: // TOGGLE SPRINT/RELAX
      if (sysState_ == PLAY)
      {
        break;
      }

      // DO SOMETHING IF IN PAUSE
      break;

    case SEC_UP: // SECONDS UP
      if (sysState_ == PLAY)
      {
        break;
      }

      // DO SOMETHING IF IN PAUSE
      break;

    case SEC_DOWN: // SECONDS DOWN
      if (sysState_ == PLAY)
      {
        break;
      }

      // DO SOMETHING IF IN PAUSE
      break;

    default:
      break;
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim10) {
    if (htim10->Instance == TIM10) {
        msCounter++;

        if(msCounter % 1000 == 0)
        {
          seconds++;
        }
    }
}

void TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim10);
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM10_Init(void)
{
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 84-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 1000-1;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_LED_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin | RUN_LED;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USER_LED_GPIO_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_PIN_Pin */
  GPIO_InitStruct.Pin = BUTTON_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);

}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}