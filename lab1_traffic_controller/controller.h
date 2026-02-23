#pragma once
#include "keypad.h"

void controller_init(void);
void controller_on_1hz_tick(void);
void controller_on_key(KeyEvent k);
int  controller_get_seconds_remaining(void);
