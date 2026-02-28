#pragma once
#include "keypad.h"

#ifdef __cplusplus
extern "C" {
#endif

void controller_init(void);
void controller_on_1hz_tick(void);
void controller_on_key(KeyEvent k);
int  controller_get_seconds_remaining(void);
void controller_task(void);

#ifdef __cplusplus
}
#endif
