#ifndef __MAIN_HPP
#define __MAIN_HPP

// Declarations for C++ code
void EventLoopCpp(void);

// Declarations for C code compatibility
#ifdef __cplusplus
extern "C" {
#endif

void EventLoopC();

#ifdef __cplusplus
}
#endif

#endif // __MAIN_HPP
