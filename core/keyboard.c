/*
 * keyboard.c
 * 
 * Copyright 2013 JS-OS <js@duck-squirell>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <system.h>

extern s32int cursor_x;

volatile s32int shift_flag = 0;
volatile s32int caps_flag = 0;

void *(*key_handle)(char);

static void keyboard_callback(registers_t regs)
{
  k_printf("\nKeypress");
}

s32int isSpecialKey(unsigned char keyPressChar)
{

  switch(keyPressChar)
  {
  case 'K': //scancode for left arrow key
    return 1;
  case 'M': //scancode for right arrow key
    return 2;
  case 'H': //scancode for up arrow key
    return 3;
  case 'P': //scancode for down arrow key
    return 4;
  case 60: /* F12 */
    reboot();
    return -1;
  default:
    return 0;
  }

}

/* KBDUS means US Keyboard Layout. This is a scancode table
 *  used to layout a standard US keyboard. I have left some
 *  comments in to give you an idea of what key is what, even
 *  though I set it's array index to 0. You can change that to
 *  whatever you want using a macro, if you wish! */
unsigned char lowerCaseKbdus[128] =
{
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\r',		/* Enter key */
  0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
  '\'', '`',   0,		/* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   'r',					/* Right shift */
  '*',
  0,	/* Alt */
  ' ',	/* Space bar */
  0,	/* Caps lock */
  0,	/* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,	/* < ... F10 */
  0,	/* 69 - Num lock*/
  0,	/* Scroll Lock */
  0,	/* Home key */
  0,	/* Up Arrow */
  0,	/* Page Up */
  '-',
  0,	/* Left Arrow */
  0,
  0,	/* Right Arrow */
  '+',
  0,	/* 79 - End key*/
  0,	/* Down Arrow */
  0,	/* Page Down */
  0,	/* Insert Key */
  0,	/* Delete Key */
  0,   0,   0,
  0,	/* F11 Key */
  0,	/* F12 Key */
  0,	/* All other keys are undefined */
};

unsigned char upperCaseKbdus[128] =
{
  0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\r',		/* Enter key */
  0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
  '\"', '~',   0,		/* Left shift */
  '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,					/* Right shift */
  '*',
  0,	/* Alt */
  ' ',	/* Space bar */
  0,	/* Caps lock */
  0,	/* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,	/* < ... F10 */
  0,	/* 69 - Num lock*/
  0,	/* Scroll Lock */
  0,	/* Home key */
  0,	/* Up Arrow */
  0,	/* Page Up */
  '-',
  0,	/* Left Arrow */
  0,
  0,	/* Right Arrow */
  '+',
  0,	/* 79 - End key*/
  0,	/* Down Arrow */
  0,	/* Page Down */
  0,	/* Insert Key */
  0,	/* Delete Key */
  0,   0,   0,
  0,	/* F11 Key */
  0,	/* F12 Key */
  0,	/* All other keys are undefined */
};

/* Handles the keyboard interrupt */
void keyboardInput_handler()
{

  unsigned char scancode;
  int speciaKeyNumber;

  /* Read from the keyboard's data buffer */
  scancode = inb(0x60);

  /*Tools used to identify scancode of non character keys, like arrows*/
  //~ k_printf("\n");
  //~ k_putChar(scancode);
  //~ k_printf("%d", scancode);
  //~ k_putChar(inb(0x60));

  /* If the top bit of the byte we read from the keyboard is
   *  set, that means that a key has just been released */
  if(scancode & 0x80)
  {
    //Key release

    //Left and right shifts
    if(scancode - 0x80 == 42 || scancode - 0x80 == 54)
      shift_flag = 0;
  }else{

    //Keypress (normal)

    //Shift
    if(scancode == 42 || scancode == 54)
    {
      shift_flag = 1;
      return;
    }
    /* Here, a key was just pressed. Please note that if you
     *  hold a key down, you will get repeated key press
     *  interrupts. */

    /* Just to show you how this works, we simply translate
     *  the keyboard scancode into an ASCII value, and then
     *  display it to the screen. You can get creative and
     *  use some flags to see if a shift is pressed and use a
     *  different layout, or you can add another 128 entries
     *  to the above layout to correspond to 'shift' being
     *  held. If shift is held using the larger lookup table,
     *  you would add 128 to the scancode when you look for it */
    speciaKeyNumber = isSpecialKey(scancode);

    if(!speciaKeyNumber) //not a special Key
    {
      if(!shift_flag) //shift is off
        key_handle(lowerCaseKbdus[scancode]);
      else if(shift_flag) //shift is on
        key_handle(upperCaseKbdus[scancode]);

    }else if(speciaKeyNumber == 1) //left arrow
      arrowKeyFunction("call", "left", &shiftCursor);
    else if(speciaKeyNumber == 2) //right arrow
      arrowKeyFunction("call", "right", &shiftCursor);
    else if(speciaKeyNumber == 3) //up arrow
      arrowKeyFunction("call", "up", &printInputBuffer);
    else if(speciaKeyNumber == 4) //down arrow
      arrowKeyFunction("call", "down", &printInputBuffer);
  }
}

/* Installs the keyboard handler into IRQ1 */
void init_keyboard()
{

  key_handle = (void*)k_putChar;

  register_interrupt_handler(IRQ1, &keyboardInput_handler);

  arrowKeyFunction("write", "left", &shiftCursor); //initialized left and right arrows to shiftCursor()
  arrowKeyFunction("write", "up", &printInputBuffer); //initialized up and down arrows to shiftCursor()

}
