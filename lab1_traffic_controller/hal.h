#pragma once

#include <stdint.h> //newly added

#ifdef __cplusplus
extern "C" {
#endif

void hal_init(void);

//NS Direction
void hal_ns_red(int on);
void hal_ns_yellow(int on);
void hal_ns_green(int on);

//EW Direction
void hal_ew_red(int on);
void hal_ew_yellow(int on);
void hal_ew_green(int on);

//Buzzer
void hal_buzzer_on(void);
void hal_buzzer_off(void);

// GPIO (used by keypad and seg7)
void hal_pin_output(uint8_t pin);
void hal_pin_input_pullup(uint8_t pin);
void hal_pin_write(uint8_t pin, uint8_t val);
uint8_t hal_pin_read(uint8_t pin);

#ifdef __cplusplus
}
#endif

