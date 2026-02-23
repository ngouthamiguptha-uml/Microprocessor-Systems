#include "timer1_1hz.h"
#include <avr/interrupt.h>

volatile bool g_tick_1hz = false;

void timer1_init_1hz(void) {
  cli();                 // stop interrupts

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // 16MHz / 1024 = 15625 counts per second
  OCR1A = 15624;

  TCCR1B |= (1 << WGM12);               // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);  // prescaler 1024
  TIMSK1 |= (1 << OCIE1A);              // compare match interrupt enable

  sei();                 // allow interrupts
}

ISR(TIMER1_COMPA_vect) {
  g_tick_1hz = true;     // set a flag; do NOT run heavy logic here
}