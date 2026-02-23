#include <Arduino.h>
#include "timer1_1hz.h"
#include "controller.h"
#include "keypad.h"
#include "seg7.h"
#include "hal.h"

void setup() {
  hal_init();
  seg7_init();
  controller_init();
  timer1_init_1hz();
}

void loop() {
  KeyEvent k = keypad_poll();
  if (k != KEY_NONE) controller_on_key(k);

  if (g_tick_1hz) {
    g_tick_1hz = false;
    controller_on_1hz_tick();
    seg7_display_number(controller_get_seconds_remaining());
  }
}
