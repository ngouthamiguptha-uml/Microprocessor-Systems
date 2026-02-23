#pragma once
#include <Arduino.h>

extern volatile bool g_tick_1hz;

void timer1_init_1hz(void);