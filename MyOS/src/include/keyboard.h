#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

/* Keyboard constants */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Special keys */
#define KEY_ESCAPE 0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_ENTER 0x1C
#define KEY_CTRL 0x1D
#define KEY_SHIFT_LEFT 0x2A
#define KEY_SHIFT_RIGHT 0x36
#define KEY_ALT 0x38
#define KEY_SPACE 0x39
#define KEY_CAPS_LOCK 0x3A

/* Function prototypes */
void keyboard_init(void);
void keyboard_handler(void);
char keyboard_getchar(void);
int keyboard_available(void);

#endif /* KEYBOARD_H */