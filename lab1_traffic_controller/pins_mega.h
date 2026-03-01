#pragma once

// NS LEDs
#define PIN_NS_RED     22
#define PIN_NS_YELLOW  24
#define PIN_NS_GREEN   26

// EW LEDs
#define PIN_EW_RED     28
#define PIN_EW_YELLOW  30
#define PIN_EW_GREEN   32

// Buzzer
#define PIN_BUZZER     34

// ====== 74HC595 (7-seg) ======
#define PIN_SR_DATA    36
#define PIN_SR_CLK     38
#define PIN_SR_LATCH   40

// Digit select pins for 4-digit display (common anode/cathode depends on module)
// You need 4 digit control pins (one per digit):
#define PIN_DIGIT_0    41
#define PIN_DIGIT_1    43
#define PIN_DIGIT_2    45
#define PIN_DIGIT_3    47

// ====== Keypad 4x4 matrix ======
// Rows as OUTPUT, Cols as INPUT_PULLUP
#define KEYPAD_R0 42
#define KEYPAD_R1 44
#define KEYPAD_R2 46
#define KEYPAD_R3 48

#define KEYPAD_C0 50
#define KEYPAD_C1 51
#define KEYPAD_C2 52
#define KEYPAD_C3 53