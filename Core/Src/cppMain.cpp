#include "cppMain.hpp"
#include "main.h"
#include "stm32f4xx_hal.h"
#include <cstdio>
#include <cstring>

volatile uint32_t msCounter = 0;
volatile uint32_t timeInSeconds_ = 0;

uint32_t startPlayTime_;
uint32_t sprintStartTime_;
uint32_t sprintStopTime_;
uint32_t restStartTime_;
uint32_t restStopTime_;

uint32_t totalSprintTime_ = 2;
uint32_t totalRestTime_ = 4;

ButtonGPIOConfig buttonConfigs[BUTTON_COUNT] = {
  {PLAY_PAUSE_BUTTON, BUTTON_GPIO_PORT},
  {TOGGLE_BUTTON, BUTTON_GPIO_PORT},
  {UP_BUTTON, BUTTON_GPIO_PORT},
  {DOWN_BUTTON, BUTTON_GPIO_PORT}
};

uint32_t bCounter[BUTTON_COUNT] = {0};
uint8_t bPressed[BUTTON_COUNT] = {NOT_PRESSED};

SystemState sysState_ = PAUSE;
ExerciseState exercisePhase_ = REST;
Sounds soundState = SPRINT_BEEP;
MenuInterval selectState_ = SELECT_SPRINT;

void EventLoopCpp(void)
{
    ButtonPollingDebounced();

    // >-----------------------------------------------------------------------------------------<
    //
    //  CHECK FOR WHETHER OR NOT TO SWAP FROM SPRINT TO REST or REST TO SPRINT
    //  
    // >-----------------------------------------------------------------------------------------<
    if(sysState_ == PLAY)
    {   

      if(((timeInSeconds_ - sprintStartTime_) >= totalSprintTime_) && (exercisePhase_ == SPRINT))
      {
        restStartTime_ = timeInSeconds_;
        sprintStopTime_ = restStartTime_;
        exercisePhase_ = REST;
        HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_Pin, GPIO_PIN_RESET);
        PlayBuzzer(&htim11, REST_BEEP);
      }

      if(((timeInSeconds_ - restStartTime_) >= totalRestTime_) && (exercisePhase_ == REST))
      {
        sprintStartTime_ = timeInSeconds_;
        restStopTime_ = sprintStartTime_;
        exercisePhase_ = SPRINT;
        HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_Pin, GPIO_PIN_SET);
        PlayBuzzer(&htim11, SPRINT_BEEP);
      }
    }

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

void PlayBuzzer(TIM_HandleTypeDef *htim, Sounds beep)
{   
    uint32_t frequency;

    if (beep == SPRINT_BEEP)
    {
        frequency = 4000;
    }
    else if (beep == REST_BEEP)
    {
        frequency = 1000;
    }
    else if (beep == PRESS_BEEP)
    {
        frequency = 200;
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
        HAL_Delay(BUZZER_LENGTH);  // Play for 500 ms
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
    else if ((pressResult == HIGH) && (bCounter[button] > COUNTER_MIN))
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

        // BEGIN SPRINT and TIMING
        restStopTime_ = timeInSeconds_;
        sprintStartTime_ = restStopTime_;
        exercisePhase_ = SPRINT;
        PlayBuzzer(&htim11,SPRINT_BEEP);

        sysState_ = PLAY;
      }
      else if (sysState_ == PLAY)
      {
        // PAUSE_MENU()
        HAL_GPIO_WritePin(USER_LED_GPIO_PORT, USER_LED_Pin, GPIO_PIN_RESET);
        sysState_ = PAUSE;

      }

      // WriteState(sysState_);

      break;
    
    case TOGGLE: // TOGGLE SPRINT/RELAX
      if (sysState_ == PLAY)
      {
        break;
      }

      ToggleSprintRestState();
      HAL_GPIO_TogglePin(USER_LED_GPIO_PORT, USER_LED_Pin);
      PlayBuzzer(&htim11, PRESS_BEEP);

      // DO SOMETHING IF IN PAUSE
      break;

    case SEC_UP: // SECONDS UP
      if (sysState_ == PLAY)
      {
        break;
      }

      IncrementIntervalInSecs();
      HAL_GPIO_TogglePin(USER_LED_GPIO_PORT, USER_LED_Pin);
      PlayBuzzer(&htim11, PRESS_BEEP);

      // DO SOMETHING IF IN PAUSE
      break;

    case SEC_DOWN: // SECONDS DOWN
      if (sysState_ == PLAY)
      {
        break;
      }

      DecrementIntervalInSecs();
      HAL_GPIO_TogglePin(USER_LED_GPIO_PORT, USER_LED_Pin);
      PlayBuzzer(&htim11, PRESS_BEEP);
      // DO SOMETHING IF IN PAUSE
      break;

    default:
      break;
  }
}

// extern "C"{
void ToggleSprintRestState(void)
{
  if (selectState_ == SELECT_SPRINT)
  {
    selectState_ = SELECT_REST;
    // Annouce Select State
  }
  else if (selectState_ == SELECT_REST)
  {
    selectState_ = SELECT_SPRINT;
    // Announce Select State
  }
}
// }

// extern "C"{
void IncrementIntervalInSecs(void)
{
  // INCREMENT
  if (selectState_ == SELECT_SPRINT)
  {
    // Announce SPRINT
    if (totalSprintTime_ < SPRINT_UP_LIM)
    {
      totalSprintTime_++;
    }

  }
  else if (selectState_ == SELECT_REST)
  {
    //Announce REST
    if (totalRestTime_ < REST_UP_LIM)
    {
      totalRestTime_++;
    }
  }

  // // APPLY  LIMITS
  // if (totalSprintTime_ > SPRINT_UP_LIM)
  // {
  //   totalSprintTime_ = SPRINT_UP_LIM;
  //   // Announce upper limit reached
  // }

  // if (totalRestTime_ > REST_UP_LIM)
  // {
  //   totalRestTime_ = REST_UP_LIM;
  //   // Announce upper limit reached
  // }
}
// }

// extern "C"{
void DecrementIntervalInSecs(void)
{
  // INCREMENT
  if (selectState_ == SELECT_SPRINT)
  {
    // Announce SPRINT
    if (totalSprintTime_ > SPRINT_LOW_LIM)
    {
      totalSprintTime_--;
    }
  }
  else if (selectState_ == SELECT_REST)
  {
    //Announce REST
    if (totalRestTime_ > REST_LOW_LIM)
    {
      totalRestTime_--;
    }
  }

  // // APPLY  LIMITS
  // if (totalSprintTime_ < SPRINT_LOW_LIM)
  // {
  //   totalSprintTime_ = SPRINT_LOW_LIM;
  //   // Announce upper limit reached
  // }

  // if (totalRestTime_ < REST_LOW_LIM)
  // {
  //   totalRestTime_ = REST_LOW_LIM;
  //   // Announce upper limit reached
  // }
}
// }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim10)
{
    if (htim10->Instance == TIM10) {
        msCounter++;

        if(msCounter % 1000 == 0)
        {
          timeInSeconds_++;

          // I dont think I need this.
          // The seconds will run for 138 years
          // if (timeInSeconds_ >= 4294967295)
          // {
          //   timeInSeconds_ = 0;
          // }
        }
    }
}

void TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim10);
}

void WriteState(SystemState state)
{
    char buffer[10];

    if (state == PLAY)
    {
      strcpy(buffer,"PLAY\r\n");
    }else if (state == PAUSE)
    {
      strcpy(buffer,"PAUSE\r\n");
    }else
    {
      strcpy(buffer, "Default\r\n");
    }

    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void WriteNum(uint32_t time)
{
    char buffer[14];
    sprintf(buffer, "%lu\r\n", (unsigned long)time);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}