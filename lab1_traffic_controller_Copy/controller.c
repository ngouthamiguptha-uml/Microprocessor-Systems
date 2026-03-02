#include <Arduino.h>
#include "controller.h"
#include "hal.h"
#include "constants.h"

typedef enum {
  ST_INIT_FLASH,          // power-up red flash until durations set AND '*' pressed
  ST_NS_GREEN,
  ST_NS_YELLOW,
  ST_NS_RED,
  ST_EW_YELLOW,           // EW yellow while NS stays red
  ST_FAILURE_FLASH,

  // buzzer “pre-change” substates
  ST_BUZZER_TO_YELLOW,
  ST_BUZZER_TO_RED,
  ST_BUZZER_TO_GREEN
} State;

static State st = ST_INIT_FLASH;

// configured durations
static int red_seconds = 24;
static int green_seconds = 20;
static bool red_set = false;
static bool green_set = false;
static bool started = false;

// countdown for current light (FSM-driven 1Hz)
static int seconds_remaining = 0;

// buzzer countdown
static int buzzer_remaining = 0;

// keypad duration entry
typedef enum { ENTRY_NONE, ENTRY_RED, ENTRY_GREEN } EntryMode;
static EntryMode entry_mode = ENTRY_NONE;
static int entry_value = 0;     // accumulating digits
static bool hash_armed = false; // for detecting double '#'

// flashing support (0.5s using millis)
static bool flash_on = false;
static unsigned long last_flash_ms = 0;

// Track flash-window entry
static bool prev_ns_green_flash_window = false;
static bool prev_ns_red_flash_window   = false;
static bool prev_ew_green_flash_window = false;

static void set_all_off(void) {
  hal_ns_red(0); hal_ns_yellow(0); hal_ns_green(0);
  hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(0);
}

// Synchronization policy (safe):
// - When NS is GREEN/YELLOW: EW is RED
// - When NS is RED: EW is GREEN
// Plus: EW gets a YELLOW phase right before switching to RED
static void apply_outputs_for_state(void) {
  switch (st) {
    case ST_INIT_FLASH:
      // EW off during init; NS red will be flashed by controller_task()
      hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_NS_GREEN:
      hal_ns_red(0); hal_ns_yellow(0); hal_ns_green(1);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    // ✅ Requirement B: during buzzer before switching to yellow, GREEN must be OFF (no steady green)
    case ST_BUZZER_TO_YELLOW:
      hal_ns_red(0); hal_ns_yellow(0); hal_ns_green(0);   // GREEN OFF during buzzer
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_NS_YELLOW:
      hal_ns_red(0); hal_ns_yellow(1); hal_ns_green(0);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_BUZZER_TO_RED:
      // keep yellow ON while buzzer runs (common behavior)
      hal_ns_red(0); hal_ns_yellow(1); hal_ns_green(0);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_NS_RED:
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(1);
      break;

    case ST_BUZZER_TO_GREEN:
      // keep NS red and EW green while buzzer runs
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_EW_YELLOW:
      // NS stays RED, EW transitions GREEN->YELLOW
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(0); hal_ew_yellow(1); hal_ew_green(0);
      break;

    case ST_FAILURE_FLASH:
      // failure: “red flashes 0.5s repeatedly”
      // We flash NS red; keep EW red ON for safety
      hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;
  }
}

static void enter_state(State next, int duration_sec) {
  st = next;
  seconds_remaining = duration_sec;
  apply_outputs_for_state();

  // reset flash timer when entering any state
  flash_on = false;
  last_flash_ms = millis();

  // reset flash-window trackers so each state transition is clean
  prev_ns_green_flash_window = false;
  prev_ns_red_flash_window = false;
  prev_ew_green_flash_window = false;
}

void controller_init(void) {
  set_all_off();

  st = ST_INIT_FLASH;
  seconds_remaining = 0;
  buzzer_remaining = 0;

  red_set = false;
  green_set = false;
  started = false;

  entry_mode = ENTRY_NONE;
  entry_value = 0;
  hash_armed = false;

  flash_on = false;
  last_flash_ms = millis();

  prev_ns_green_flash_window = false;
  prev_ns_red_flash_window = false;
  prev_ew_green_flash_window = false;

  apply_outputs_for_state();
}

int controller_get_seconds_remaining(void) {
  if (st == ST_BUZZER_TO_YELLOW || st == ST_BUZZER_TO_RED || st == ST_BUZZER_TO_GREEN) {
    return buzzer_remaining;
  }
  return seconds_remaining;
}

// 0.5s flashing implemented here using millis() (non-blocking)
void controller_task(void) {
  unsigned long now = millis();

  // INIT: 1s flashing NS red
  if (st == ST_INIT_FLASH) {
    if (now - last_flash_ms >= 1000) {
      last_flash_ms = now;
      flash_on = !flash_on;
      hal_ns_red(flash_on ? 1 : 0);
    }
    return;
  }

  // FAILURE: 0.5s flashing NS red
  if (st == ST_FAILURE_FLASH) {
    if (now - last_flash_ms >= FLASH_HALF_PERIOD_MS) {
      last_flash_ms = now;
      flash_on = !flash_on;
      hal_ns_red(flash_on ? 1 : 0);
    }
    return;
  }

  // Flash windows = last 3 seconds (or FLASH_WINDOW_SECONDS) while > 0
  bool ns_green_flash_window =
      (st == ST_NS_GREEN) &&
      (seconds_remaining <= FLASH_WINDOW_SECONDS) &&
      (seconds_remaining > 0);

  bool ns_red_flash_window =
      (st == ST_NS_RED) &&
      (seconds_remaining <= FLASH_WINDOW_SECONDS) &&
      (seconds_remaining > 0);

  bool ew_green_flash_window =
      (st == ST_NS_RED) &&
      (seconds_remaining <= FLASH_WINDOW_SECONDS) &&
      (seconds_remaining > 0);

  // ---- NS GREEN FLASH (no restore-to-ON after window) ----
  if (ns_green_flash_window && !prev_ns_green_flash_window) {
    flash_on = true;      // start ON
    hal_ns_green(1);
    last_flash_ms = now;  // reset cadence
  }
  if (ns_green_flash_window) {
    if (now - last_flash_ms >= FLASH_HALF_PERIOD_MS) {
      last_flash_ms = now;
      flash_on = !flash_on;
      hal_ns_green(flash_on ? 1 : 0);
    }
  }

  // ---- NS RED FLASH ----
  if (ns_red_flash_window && !prev_ns_red_flash_window) {
    flash_on = true;      // start ON
    hal_ns_red(1);
    last_flash_ms = now;
  }
  if (ns_red_flash_window) {
    if (now - last_flash_ms >= FLASH_HALF_PERIOD_MS) {
      last_flash_ms = now;
      flash_on = !flash_on;
      hal_ns_red(flash_on ? 1 : 0);
    }
  }

  // ---- EW GREEN FLASH (mirror same cadence; no restore-to-ON) ----
  if (ew_green_flash_window && !prev_ew_green_flash_window) {
    hal_ew_green(1);
  }
  if (ew_green_flash_window) {
    hal_ew_green(flash_on ? 1 : 0);
  }

  prev_ns_green_flash_window = ns_green_flash_window;
  prev_ns_red_flash_window = ns_red_flash_window;
  prev_ew_green_flash_window = ew_green_flash_window;
}

static int clamp_duration(int v) {
  if (v < MIN_DURATION_SECONDS) return MIN_DURATION_SECONDS;
  if (v > 999) return 999;
  return v;
}

// Called once per second by Timer1 tick flag
void controller_on_1hz_tick(void) {
  // Buzzer substates count down first
  if (st == ST_BUZZER_TO_YELLOW || st == ST_BUZZER_TO_RED || st == ST_BUZZER_TO_GREEN) {
    if (buzzer_remaining > 0) buzzer_remaining--;
    if (buzzer_remaining == 0) {
      hal_buzzer_off();

      if (st == ST_BUZZER_TO_YELLOW) {
        enter_state(ST_NS_YELLOW, YELLOW_SECONDS);
      } else if (st == ST_BUZZER_TO_RED) {
        enter_state(ST_NS_RED, red_seconds);
      } else { // ST_BUZZER_TO_GREEN
        // Insert EW_YELLOW for 3 seconds (NS stays RED)
        enter_state(ST_EW_YELLOW, YELLOW_SECONDS);
      }
    }
    return;
  }

  // Main states countdown
  if (st == ST_NS_GREEN || st == ST_NS_YELLOW || st == ST_NS_RED || st == ST_EW_YELLOW) {
    if (seconds_remaining > 0) seconds_remaining--;

    if (seconds_remaining == 0) {

      // If EW yellow finished, go straight to NS green (NO buzzer here)
      if (st == ST_EW_YELLOW) {
        enter_state(ST_NS_GREEN, green_seconds);
        return;
      }

      // Before changing a light: buzzer 3 seconds
      hal_buzzer_on();
      buzzer_remaining = BUZZER_SECONDS;

      if (st == ST_NS_GREEN) {
        st = ST_BUZZER_TO_YELLOW;
      } else if (st == ST_NS_YELLOW) {
        st = ST_BUZZER_TO_RED;
      } else { // ST_NS_RED
        st = ST_BUZZER_TO_GREEN;
      }

      apply_outputs_for_state();
    }
    return;
  }

  // init/failure states are handled via controller_task() + key events
}

// Key input handling (durations + start + failure)
void controller_on_key(KeyEvent k) {
  // detect double '#'
  if (k == KEY_HASH) {
    if (hash_armed) {
      // double hash => failure mode ONLY from normal operation
      if (st != ST_INIT_FLASH && st != ST_FAILURE_FLASH) {
        enter_state(ST_FAILURE_FLASH, 0);
      }
      hash_armed = false;
    } else {
      hash_armed = true;
    }
  } else {
    hash_armed = false;
  }

  // In FAILURE mode: durations setting should exit failure and go back to init flashing
  if (st == ST_FAILURE_FLASH) {
    if (k == KEY_A || k == KEY_B) {
      started = false;
      enter_state(ST_INIT_FLASH, 0);
    } else {
      return;
    }
  }

  // Duration entry mode selection
  if (k == KEY_A) {
    entry_mode = ENTRY_RED;
    entry_value = 0;
    return;
  }
  if (k == KEY_B) {
    entry_mode = ENTRY_GREEN;
    entry_value = 0;
    return;
  }

  // Start operation only when both durations set and '*' pressed
  if (k == KEY_STAR) {
    if (red_set && green_set) {
      started = true;
      enter_state(ST_NS_RED, red_seconds);
    }
    return;
  }

  // Digit accumulation
  int digit = -1;
  switch (k) {
    case KEY_0: digit = 0; break;
    case KEY_1: digit = 1; break;
    case KEY_2: digit = 2; break;
    case KEY_3: digit = 3; break;
    case KEY_4: digit = 4; break;
    case KEY_5: digit = 5; break;
    case KEY_6: digit = 6; break;
    case KEY_7: digit = 7; break;
    case KEY_8: digit = 8; break;
    case KEY_9: digit = 9; break;
    default: break;
  }

  if (digit >= 0 && entry_mode != ENTRY_NONE) {
    entry_value = entry_value * 10 + digit;
    if (entry_value > 999) entry_value = 999;
    return;
  }

  // Confirm entry with '#'
  if (k == KEY_HASH && entry_mode != ENTRY_NONE) {
    int v = clamp_duration(entry_value);

    if (entry_mode == ENTRY_RED) {
      red_seconds = v;
      red_set = true;
    } else if (entry_mode == ENTRY_GREEN) {
      green_seconds = v;
      green_set = true;
    }

    entry_mode = ENTRY_NONE;
    entry_value = 0;

    // Stay in init flashing until '*' is pressed after both are set
    if (!started) {
      enter_state(ST_INIT_FLASH, 0);
    }
    return;
  }
}