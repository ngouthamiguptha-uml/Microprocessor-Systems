#ifndef SEG7_H
#define SEG7_H

#include "pins_mega.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Call once in setup()
void seg7Init(void);

// Set the value to display (0–99, last 2 digits only)
void seg7SetValue(int value);

// Call as fast as possible in loop() — handles multiplexing
void seg7Refresh(void);

#ifdef __cplusplus
}
#endif

#endif // SEG7_H