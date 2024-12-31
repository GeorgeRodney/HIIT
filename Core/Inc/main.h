/**
******************************************************************************
* @file           : main.h
* @brief          : Header for main.c file.
*                   This file contains the common defines of the application.
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

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include "stm32f401xc.h"


void Error_Handler(void);

#define USER_LED_Pin GPIO_PIN_14
#define RUN_LED GPIO_PIN_15
#define USER_LED_GPIO_PORT GPIOC
#define BUTTON_PIN GPIO_PIN_0
#define BUTTON_GPIO_PORT GPIOB

#define LOW 0
#define HIGH 1
#define BUTTON_COUNT 1
#define PRESSED 1
#define NOT_PRESSED 0
#define PRESSED_COUNT 12500
#define NOT_PRESSED_COUNT 2500
#define COUNTER_MAX 15000
#define COUNTER_MIN 0

extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

typedef struct {
  uint16_t pin;
  GPIO_TypeDef* port;
} ButtonGPIOConfig;

typedef enum {
  PLAY = 0,
  PAUSE
} SystemState;

typedef enum {
  PLAY_PAUSE = 0,
  TOGGLE,
  SEC_UP,
  SEC_DOWN
} Buttons;

typedef enum {
  SPRINT = 0,
  REST
} Sounds;

extern ButtonGPIOConfig buttonConfigs[BUTTON_COUNT];

extern int32_t  bCounter[BUTTON_COUNT];
extern uint8_t  bPressed[BUTTON_COUNT];

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
