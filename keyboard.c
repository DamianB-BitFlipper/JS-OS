#include "keyboard.h"
#include "isr.h"

#include "k_math.h"
#include "k_stdio.h"
#include "k_shell.h"

//~ #include "task.h"
#include "syscall.h"

extern int cursor_x;

static void keyboard_callback(registers_t regs)
{
  k_printf("\nKeypress");
}

int isSpecialKey(unsigned char keyPressChar)
{

  if(keyPressChar == 'K') //scancode for left arrow key
  {
    return 1;
  }else if(keyPressChar == 'M') //scancode for right arrow key
  {
    return 2;
  }else if(keyPressChar == 'H') //scancode for up arrow key
  {
    return 3;
  }else if(keyPressChar == 'P') //scancode for down arrow key
  {
    return 4;
  }else{
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
  //~ k_putChar(inb(0x60));
   
  /* If the top bit of the byte we read from the keyboard is
  *  set, that means that a key has just been released */
  if (scancode & 0x80)
  {
    //~ if(scancode = '*')
    //~ {
      //~ k_putChar(scancode);
    //~ }
    /* You can use this one to see if the user released the
    *  shift, alt, or control keys... */
  }
  else
  {
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

    if(speciaKeyNumber == 0) //not a special Key
    {
      k_putChar(lowerCaseKbdus[scancode]);
      //~ syscall_user_putChar(lowerCaseKbdus[scancode]);
    }else if(speciaKeyNumber == 1) //left arrow
    {
      //~ k_printf("<--");
      //~ shiftCursor(-1);
      arrowKeyFunction("call", "left", &shiftCursor);
      //~ arrowKeyFunction("call", "left", &notmalCursor);
    }else if(speciaKeyNumber == 2) //right arrow
    {
      arrowKeyFunction("call", "right", &shiftCursor);
      //~ arrowKeyFunction("call", "right", &notmalCursor);
    }else if(speciaKeyNumber == 3) //up arrow
    {
      //~ printInputBuffer(1);
      arrowKeyFunction("call", "up", &printInputBuffer);
      //~ arrowKeyFunction("call", "up", &notmalCursor);
    }else if(speciaKeyNumber == 4) //down arrow
    {
      //~ printInputBuffer(-1);
      arrowKeyFunction("call", "down", &printInputBuffer);
      //~ arrowKeyFunction("call", "down", &notmalCursor);
    }
    //~ k_printf("\nKeyPress");
  }
}

/* Installs the keyboard handler into IRQ1 */
void init_keyboard()
{
    
  register_interrupt_handler(IRQ1, &keyboardInput_handler);

  arrowKeyFunction("write", "left", &shiftCursor); //initialized left and right arrows to shiftCursor()
  arrowKeyFunction("write", "up", &printInputBuffer); //initialized up and down arrows to shiftCursor()
  //~ arrowKeyFunction("write", "left", &notmalCursor); //initialized left and right arrows to shiftCursor()
  //~ arrowKeyFunction("write", "up", &normalVCursor); //initialized up and down arrows to shiftCursor()
  
}
