#include <Arduino.h>
#include "timer1_1hz.h"
#include "hal.h"
#include "controller.h"

void setup() {
  hal_init();
  controller_init();
  timer1_init_1hz();
}

void loop() {
  controller_task();

  if (g_tick_1hz) {
    g_tick_1hz = false;
    controller_on_1hz_tick();
  }
}