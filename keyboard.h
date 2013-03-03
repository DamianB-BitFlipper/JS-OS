#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "common.h"

/*Initialize the keyboard*/
void init_keyboard();

/*function that checks for special keys, up, down, left, right, etc*/
int isSpecialKey(unsigned char keyPressChar);

#endif
