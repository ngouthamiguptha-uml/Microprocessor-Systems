#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  KEY_NONE,
  KEY_STAR,
  KEY_HASH,
  KEY_A, KEY_B,
  KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9
} KeyEvent;

void keypad_init(void);
KeyEvent keypad_poll(void);

#ifdef __cplusplus
}
#endif