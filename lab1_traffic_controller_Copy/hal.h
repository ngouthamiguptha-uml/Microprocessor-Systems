#pragma once
#include <stdint.h>

// Hardware abstraction layer for the traffic controller.
// Mixed C + AVR assembly (ATmega2560 / Arduino Mega).

#ifdef __cplusplus
extern "C" {
#endif

// Assembly: configure LED + buzzer pins as outputs (DDR registers)
void traffic_gpio_init_asm(void);

// Assembly: set one output ON/OFF by logical id
void traffic_set_output_asm(uint8_t id, uint8_t on);

// Logical output ids (keep in sync with traffic_io.S)
typedef enum {
  OUT_NS_RED = 0,
  OUT_NS_YELLOW = 1,
  OUT_NS_GREEN = 2,
  OUT_EW_RED = 3,
  OUT_EW_YELLOW = 4,
  OUT_EW_GREEN = 5,
  OUT_BUZZER = 6
} TrafficOutputId;

// C-facing helpers used by controller.c / .ino
void hal_init(void);

void hal_ns_red(uint8_t on);
void hal_ns_yellow(uint8_t on);
void hal_ns_green(uint8_t on);

void hal_ew_red(uint8_t on);
void hal_ew_yellow(uint8_t on);
void hal_ew_green(uint8_t on);

void hal_buzzer_on(void);
void hal_buzzer_off(void);

#ifdef __cplusplus
} // extern "C"
#endif
