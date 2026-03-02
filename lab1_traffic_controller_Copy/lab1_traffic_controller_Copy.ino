#include <Arduino.h>
#include "timer1_1hz.h"
#include "hal.h"
#include "controller.h"
#include "keypad.h"
#include "seg7.h"

void setup() {
    hal_init();
    keypad_init();
    seg7Init();
    seg7SetValue(0);
    controller_init();
    timer1_init_1hz();
}

void loop() {
    // 7-seg multiplex refresh
    seg7Refresh();

    // Keypad polling
    KeyEvent k = keypad_poll();
    if (k != KEY_NONE) {
        controller_on_key(k);
    }

    // Flashing logic (0.5s) and init/failure blink
    controller_task();

    // 1Hz FSM tick
    if (g_tick_1hz) {
        g_tick_1hz = false;
        controller_on_1hz_tick();

        // Display seconds remaining on 7-seg (last 2 digits)
        seg7SetValue(controller_get_seconds_remaining());
    }
}
