#pragma once
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile bool g_tick_1hz;
void timer1_init_1hz(void);

#ifdef __cplusplus
}
#endif