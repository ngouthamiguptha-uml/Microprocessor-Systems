#include <Arduino.h>
#include "controller.h"
#include "hal.h"
#include "constants.h"

// =====================
// FSM states (2-direction)
// =====================
typedef enum {
  ST_EW_GREEN,          // EW moves, NS stopped
  ST_EW_YELLOW,
  ST_ALL_RED_AFTER_EW,  // clearance (both red)
  ST_NS_GREEN,          // NS moves, EW stopped
  ST_NS_YELLOW,
  ST_ALL_RED_AFTER_NS,  // clearance (both red)

  // buzzer sub-states (3s warning before each change)
  ST_BUZZER_TO_EW_YELLOW,
  ST_BUZZER_TO_ALL_RED_AFTER_EW,
  ST_BUZZER_TO_NS_GREEN,

  ST_BUZZER_TO_NS_YELLOW,
  ST_BUZZER_TO_ALL_RED_AFTER_NS,
  ST_BUZZER_TO_EW_GREEN
} State;

static State st = ST_EW_GREEN;

// durations (can be overridden via keypad later)
static int red_seconds   = 24;
static int green_seconds = 20;

// computed clearance so "red total" can equal red_seconds
static int all_red_seconds = 0;

// countdowns
static int seconds_remaining = 0;
static int buzzer_remaining  = 0;

// flashing support
static bool flash_on = false;
static unsigned long last_flash_ms = 0;

// ---------------------
// helpers
// ---------------------
static void set_all_off(void) {
  hal_ns_red(0); hal_ns_yellow(0); hal_ns_green(0);
  hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(0);
  hal_buzzer_off();
}

// Apply outputs for the "displayed" phase.
// Note: buzzer states keep the SAME outputs as the phase they correspond to.
static void apply_outputs(void) {
  switch (st) {
    // EW moving phases (NS stopped)
    case ST_EW_GREEN:
    case ST_BUZZER_TO_EW_YELLOW:
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(1);
      break;

    case ST_EW_YELLOW:
    case ST_BUZZER_TO_ALL_RED_AFTER_EW:
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(0); hal_ew_yellow(1); hal_ew_green(0);
      break;

    case ST_ALL_RED_AFTER_EW:
    case ST_BUZZER_TO_NS_GREEN:
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    // NS moving phases (EW stopped)
    case ST_NS_GREEN:
    case ST_BUZZER_TO_NS_YELLOW:
      hal_ns_red(0); hal_ns_yellow(0); hal_ns_green(1);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_NS_YELLOW:
    case ST_BUZZER_TO_ALL_RED_AFTER_NS:
      hal_ns_red(0); hal_ns_yellow(1); hal_ns_green(0);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;

    case ST_ALL_RED_AFTER_NS:
    case ST_BUZZER_TO_EW_GREEN:
      hal_ns_red(1); hal_ns_yellow(0); hal_ns_green(0);
      hal_ew_red(1); hal_ew_yellow(0); hal_ew_green(0);
      break;
  }
}

static void enter_state(State next, int duration_sec) {
  st = next;
  seconds_remaining = duration_sec;
  flash_on = false;
  last_flash_ms = millis();
  apply_outputs();
}

static bool is_buzzer_state(State s) {
  return (s == ST_BUZZER_TO_EW_YELLOW ||
          s == ST_BUZZER_TO_ALL_RED_AFTER_EW ||
          s == ST_BUZZER_TO_NS_GREEN ||
          s == ST_BUZZER_TO_NS_YELLOW ||
          s == ST_BUZZER_TO_ALL_RED_AFTER_NS ||
          s == ST_BUZZER_TO_EW_GREEN);
}

// =====================
// Public API
// =====================
void controller_init(void) {
  set_all_off();

  // clearance to make total red time = red_seconds
  // (red_seconds = green_seconds + YELLOW_SECONDS + all_red_seconds)
  all_red_seconds = red_seconds - green_seconds - YELLOW_SECONDS;
  if (all_red_seconds < 0) all_red_seconds = 0;

  // Start with EW moving (NS red)
  enter_state(ST_EW_GREEN, green_seconds);
}

int controller_get_seconds_remaining(void) {
  if (is_buzzer_state(st)) return buzzer_remaining;
  return seconds_remaining;
}

void controller_on_key(KeyEvent k) {
  (void)k;
}

// Flash last 3 seconds of:
// - the currently active GREEN
// - the currently active RED (the stopped direction), during its last 3 seconds
void controller_task(void) {
  unsigned long now = millis();

  // Only flash near the end of a normal (non-buzzer) phase.
  if (is_buzzer_state(st)) return;

  bool in_flash_window =
    (seconds_remaining > 0 && seconds_remaining <= FLASH_WINDOW_SECONDS);

  if (!in_flash_window) return;

  if (now - last_flash_ms >= FLASH_HALF_PERIOD_MS) {
    last_flash_ms = now;
    flash_on = !flash_on;

    // Flash rules per spec: flash RED and GREEN in their last 3 seconds.
    // Which lamp is "the red" depends on which direction is stopped.

    if (st == ST_EW_GREEN) {
      // EW green is active; NS red is active
      hal_ew_green(flash_on ? 1 : 0);
      hal_ns_red(flash_on ? 1 : 0);
    } else if (st == ST_NS_GREEN) {
      // NS green is active; EW red is active
      hal_ns_green(flash_on ? 1 : 0);
      hal_ew_red(flash_on ? 1 : 0);
    } else if (st == ST_EW_YELLOW) {
      // no flash on yellow by spec
    } else if (st == ST_NS_YELLOW) {
      // no flash on yellow by spec
    } else if (st == ST_ALL_RED_AFTER_EW || st == ST_ALL_RED_AFTER_NS) {
      // both red; if you want, you can flash both reds here, but spec doesn't require it
    }
  }
}

// 1Hz tick drives state transitions + buzzer delays
void controller_on_1hz_tick(void) {
  // Buzzer sub-state behavior: count buzzer, then enter the real next phase.
  if (is_buzzer_state(st)) {
    if (buzzer_remaining > 0) buzzer_remaining--;

    if (buzzer_remaining == 0) {
      hal_buzzer_off();

      // After buzzer, jump to the next *real* phase
      if (st == ST_BUZZER_TO_EW_YELLOW) {
        enter_state(ST_EW_YELLOW, YELLOW_SECONDS);
      } else if (st == ST_BUZZER_TO_ALL_RED_AFTER_EW) {
        enter_state(ST_ALL_RED_AFTER_EW, all_red_seconds);
      } else if (st == ST_BUZZER_TO_NS_GREEN) {
        enter_state(ST_NS_GREEN, green_seconds);
      } else if (st == ST_BUZZER_TO_NS_YELLOW) {
        enter_state(ST_NS_YELLOW, YELLOW_SECONDS);
      } else if (st == ST_BUZZER_TO_ALL_RED_AFTER_NS) {
        enter_state(ST_ALL_RED_AFTER_NS, all_red_seconds);
      } else { // ST_BUZZER_TO_EW_GREEN
        enter_state(ST_EW_GREEN, green_seconds);
      }
    }
    return;
  }

  // Normal phase countdown
  if (seconds_remaining > 0) seconds_remaining--;

  if (seconds_remaining == 0) {
    // Requirement (8): beep 3 seconds before a light is changed
    hal_buzzer_on();
    buzzer_remaining = BUZZER_SECONDS;

    // Move into appropriate buzzer state (outputs remain the same)
    if (st == ST_EW_GREEN) {
      st = ST_BUZZER_TO_EW_YELLOW;
    } else if (st == ST_EW_YELLOW) {
      st = ST_BUZZER_TO_ALL_RED_AFTER_EW;
    } else if (st == ST_ALL_RED_AFTER_EW) {
      st = ST_BUZZER_TO_NS_GREEN;
    } else if (st == ST_NS_GREEN) {
      st = ST_BUZZER_TO_NS_YELLOW;
    } else if (st == ST_NS_YELLOW) {
      st = ST_BUZZER_TO_ALL_RED_AFTER_NS;
    } else { // ST_ALL_RED_AFTER_NS
      st = ST_BUZZER_TO_EW_GREEN;
    }

    apply_outputs(); // keep current lights during buzzer
  }
}