#include "cppMain.hpp"
#include "main.h"
#include "stm32f4xx_hal.h"


void ButtonPollingDebounced(void);
void ExecutePress(Buttons button);

// TIM_HandleTypeDef htim10;
volatile uint32_t msCounter = 0;  // Millisecond counter
volatile uint32_t seconds = 0;

ButtonGPIOConfig buttonConfigs[BUTTON_COUNT] = {
  {BUTTON_PIN, BUTTON_GPIO_PORT}
};

int32_t bCounter[BUTTON_COUNT] = {0};
uint8_t bPressed[BUTTON_COUNT] = {NOT_PRESSED};

SystemState sysState_ = PAUSE;

void EventLoopCpp(void)
{
    ButtonPollingDebounced();
}

extern "C"
{
    void EventLoopC()
    {
        HAL_TIM_Base_Start_IT(&htim10);

        while(true)
        {
            EventLoopCpp();
        }
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

  for(uint8_t button = PLAY_PAUSE; button < BUTTON_COUNT; button++)
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
      ExecutePress(static_cast<Buttons>(button));
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