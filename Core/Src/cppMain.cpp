#include "cppMain.hpp"
#include "main.h"
#include "stm32f4xx_hal.h"


void ButtonPollingDebounced(void);
void ExecutePress(Buttons button);
void PlayBuzzer(TIM_HandleTypeDef* htim, Sounds beep);

volatile uint32_t msCounter = 0;
volatile uint32_t seconds = 0;
uint32_t prevTime_;

ButtonGPIOConfig buttonConfigs[BUTTON_COUNT] = {
  {BUTTON_PIN, BUTTON_GPIO_PORT}
};

int32_t bCounter[BUTTON_COUNT] = {0};
uint8_t bPressed[BUTTON_COUNT] = {NOT_PRESSED};

SystemState sysState_ = PAUSE;

void EventLoopCpp(void)
{
    ButtonPollingDebounced();

    if(sysState_ == PLAY)
    {
        if((seconds - prevTime_) >= 3)
        {
            PlayBuzzer(&htim11, SPRINT);
            prevTime_ = seconds;
        }
    }
}

extern "C"
{
    void EventLoopC()
    {
        HAL_TIM_Base_Start_IT(&htim10);
        prevTime_ = seconds;

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

void PlayBuzzer(TIM_HandleTypeDef *htim, Sounds beep)
{   
    uint32_t frequency;

    if (beep == SPRINT)
    {
        frequency = 2000;
    }
    else if (beep == REST)
    {
        frequency = 500;
    }

    // Update the Timer Period for the desired frequency
    uint32_t timerClock = 84000000;  // 84 MHz APB1 Timer Clock
    uint32_t prescaler = htim->Init.Prescaler + 1;
    uint32_t period = (timerClock / (prescaler * frequency)) - 1;

    // Check validity and apply
    if (period <= 0xFFFF)  // Ensure within 16-bit timer range
    {
        __HAL_TIM_SET_AUTORELOAD(htim, period);
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, period / 2);  // 50% Duty Cycle
        HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);  // Start PWM

        HAL_Delay(100);  // Play for 500 ms

        HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);  // Stop PWM
    }
}

void ButtonPollingDebounced(void)
{

  for(Buttons button = PLAY_PAUSE; button < BUTTON_COUNT; button = static_cast<Buttons>(button + 1))
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