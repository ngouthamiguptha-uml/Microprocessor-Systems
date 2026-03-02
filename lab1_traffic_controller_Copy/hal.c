#include "hal.h"

// Level A requirement:
// - GPIO init and output toggling done in AVR assembly (traffic_io.S)
// - FSM/timing remains in C

static inline void set_out(TrafficOutputId id, uint8_t on) {
  traffic_set_output_asm((uint8_t)id, on ? 1 : 0);
}

void hal_init(void) {
  traffic_gpio_init_asm();

  // Start in a safe "all off" state (controller will turn on required lights)
  set_out(OUT_NS_RED, 0);
  set_out(OUT_NS_YELLOW, 0);
  set_out(OUT_NS_GREEN, 0);

  set_out(OUT_EW_RED, 0);
  set_out(OUT_EW_YELLOW, 0);
  set_out(OUT_EW_GREEN, 0);

  set_out(OUT_BUZZER, 0);
}

void hal_ns_red(uint8_t on)    { set_out(OUT_NS_RED, on); }
void hal_ns_yellow(uint8_t on) { set_out(OUT_NS_YELLOW, on); }
void hal_ns_green(uint8_t on)  { set_out(OUT_NS_GREEN, on); }

void hal_ew_red(uint8_t on)    { set_out(OUT_EW_RED, on); }
void hal_ew_yellow(uint8_t on) { set_out(OUT_EW_YELLOW, on); }
void hal_ew_green(uint8_t on)  { set_out(OUT_EW_GREEN, on); }

void hal_buzzer_on(void)  { set_out(OUT_BUZZER, 1); }
void hal_buzzer_off(void) { set_out(OUT_BUZZER, 0); }
