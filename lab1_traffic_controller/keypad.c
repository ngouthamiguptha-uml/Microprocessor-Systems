#include <Arduino.h>
#include "keypad.h"
#include "pins_mega.h"

static const uint8_t ROWS[4] = {KEYPAD_R0, KEYPAD_R1, KEYPAD_R2, KEYPAD_R3};
static const uint8_t COLS[4] = {KEYPAD_C0, KEYPAD_C1, KEYPAD_C2, KEYPAD_C3};

// Typical 4x4 keypad layout:
// [1][2][3][A]
// [4][5][6][B]
// [7][8][9][C]  (C unused here)
// [*][0][#][D]  (D unused here)
static const KeyEvent KEYMAP[4][4] = {
  {KEY_1, KEY_2, KEY_3, KEY_A},
  {KEY_4, KEY_5, KEY_6, KEY_B},
  {KEY_7, KEY_8, KEY_9, KEY_NONE}, // C -> KEY_NONE
  {KEY_STAR, KEY_0, KEY_HASH, KEY_NONE} // D -> KEY_NONE
};

void keypad_init(void) {
  for (int r = 0; r < 4; r++) {
    pinMode(ROWS[r], OUTPUT);
    digitalWrite(ROWS[r], HIGH);
  }
  for (int c = 0; c < 4; c++) {
    pinMode(COLS[c], INPUT_PULLUP);
  }
}

static KeyEvent scan_once(void) {
  for (int r = 0; r < 4; r++) {
    // drive one row low, others high
    for (int rr = 0; rr < 4; rr++) digitalWrite(ROWS[rr], HIGH);
    digitalWrite(ROWS[r], LOW);

    delayMicroseconds(5);

    for (int c = 0; c < 4; c++) {
      if (digitalRead(COLS[c]) == LOW) {
        return KEYMAP[r][c];
      }
    }
  }
  return KEY_NONE;
}

// Simple debounce: return key only when it becomes pressed, then wait for release.
KeyEvent keypad_poll(void) {
  static KeyEvent last_reported = KEY_NONE;
  static unsigned long last_change_ms = 0;

  KeyEvent k = scan_once();
  unsigned long now = millis();

  // debounce window
  if (k != last_reported && (now - last_change_ms) > 40) {
    last_change_ms = now;
    last_reported = k;

    // Only report non-NONE presses; ignore release transitions
    if (k != KEY_NONE) return k;
  }

  return KEY_NONE;
}