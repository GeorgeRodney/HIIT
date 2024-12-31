#ifndef __MAIN_HPP
#define __MAIN_HPP

#include "main.h"

// Declarations for C++ code
void EventLoopCpp(void);
void ButtonPollingDebounced(void);
void ExecutePress(Buttons button);
void PlayBuzzer(TIM_HandleTypeDef* htim, Sounds beep);
void WriteState(SystemState state);
void WriteTime(uint32_t time);

// Declarations for C code compatibility
#ifdef __cplusplus
extern "C" {
#endif

void EventLoopC();

#ifdef __cplusplus
}
#endif

#endif // __MAIN_HPP
