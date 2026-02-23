#pragma once

void hal_init(void);

// NS direction
void hal_ns_red(int on);
void hal_ns_yellow(int on);
void hal_ns_green(int on);

// EW direction
void hal_ew_red(int on);
void hal_ew_yellow(int on);
void hal_ew_green(int on);

void hal_buzzer_on(void);
void hal_buzzer_off(void);
