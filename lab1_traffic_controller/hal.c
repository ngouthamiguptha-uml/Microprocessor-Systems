#include <Arduino.h>
#include "hal.h"
#include "pins_mega.h"

void hal_init(void) {
  pinMode(PIN_NS_RED, OUTPUT);    pinMode(PIN_NS_YELLOW, OUTPUT); pinMode(PIN_NS_GREEN, OUTPUT);
  pinMode(PIN_EW_RED, OUTPUT);    pinMode(PIN_EW_YELLOW, OUTPUT); pinMode(PIN_EW_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_DIGIT_0, OUTPUT);
  pinMode(PIN_DIGIT_1, OUTPUT);
  pinMode(PIN_DIGIT_2, OUTPUT);
  pinMode(PIN_DIGIT_3, OUTPUT);

  // Default off
  hal_ns_red(0); hal_ns_yellow(0); hal_ns_green(0);
  hal_ew_red(0); hal_ew_yellow(0); hal_ew_green(0);
  hal_buzzer_off();
}

void hal_ns_red(int on)    { digitalWrite(PIN_NS_RED,    on ? HIGH : LOW); }
void hal_ns_yellow(int on) { digitalWrite(PIN_NS_YELLOW, on ? HIGH : LOW); }
void hal_ns_green(int on)  { digitalWrite(PIN_NS_GREEN,  on ? HIGH : LOW); }

void hal_ew_red(int on)    { digitalWrite(PIN_EW_RED,    on ? HIGH : LOW); }
void hal_ew_yellow(int on) { digitalWrite(PIN_EW_YELLOW, on ? HIGH : LOW); }
void hal_ew_green(int on)  { digitalWrite(PIN_EW_GREEN,  on ? HIGH : LOW); }

void hal_buzzer_on(void)   { digitalWrite(PIN_BUZZER, HIGH); }
void hal_buzzer_off(void)  { digitalWrite(PIN_BUZZER, LOW); }