#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "lib/main.h"
typedef struct {
    uint8_t scancode;
    uint8_t asciicode;
} out_key;
extern void irq1code();
extern out_key waitget_key();
#endif