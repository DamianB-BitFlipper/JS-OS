#include "mouse.h"
#include "isr.h"

#include "k_math.h"
#include "k_stdio.h"
#include "k_shell.h"


/* Handles the mouse interrupt */
void mouseInput_handler()
{

  k_printf("\tmoved");
  //unsigned char scancode;
  //int speciaKeyNumber;

  ///* Read from the keyboard's data buffer */
  //scancode = inb(0x60);

  ///*Tools used to identify scancode of non character keys, like arrows*/
  ////~ k_printf("\n");
  ////~ k_putChar(scancode);
   
  ///* If the top bit of the byte we read from the keyboard is
  //*  set, that means that a key has just been released */
  //if (scancode & 0x80)
  //{
    ////~ if(scancode = '*')
    ////~ {
      ////~ k_putChar(scancode);
    ////~ }
    ///* You can use this one to see if the user released the
    //*  shift, alt, or control keys... */
  //}
  //else
  //{
    ///* Here, a key was just pressed. Please note that if you
    //*  hold a key down, you will get repeated key press
    //*  interrupts. */

    ///* Just to show you how this works, we simply translate
    //*  the keyboard scancode into an ASCII value, and then
    //*  display it to the screen. You can get creative and
    //*  use some flags to see if a shift is pressed and use a
    //*  different layout, or you can add another 128 entries
    //*  to the above layout to correspond to 'shift' being
    //*  held. If shift is held using the larger lookup table,
    //*  you would add 128 to the scancode when you look for it */
    //speciaKeyNumber = isSpecialKey(scancode);

    //if(speciaKeyNumber == 0) //not a special Key
    //{
      //k_putChar(lowerCaseKbdus[scancode]);
    //}else if(speciaKeyNumber == 1) //left arrow
    //{
      ////~ k_printf("<--");
      ////~ shiftCursor(-1);
      //arrowKeyFunction("call", "left", &shiftCursor);
      ////~ arrowKeyFunction("call", "left", &notmalCursor);
    //}else if(speciaKeyNumber == 2) //right arrow
    //{
      //arrowKeyFunction("call", "right", &shiftCursor);
      ////~ arrowKeyFunction("call", "right", &notmalCursor);
    //}else if(speciaKeyNumber == 3) //up arrow
    //{
      ////~ printInputBuffer(1);
      //arrowKeyFunction("call", "up", &printInputBuffer);
      ////~ arrowKeyFunction("call", "up", &notmalCursor);
    //}else if(speciaKeyNumber == 4) //down arrow
    //{
      ////~ printInputBuffer(-1);
      //arrowKeyFunction("call", "down", &printInputBuffer);
      ////~ arrowKeyFunction("call", "down", &notmalCursor);
    //}
    ////~ k_printf("\nKeyPress");
  //}
}

/* Installs the mouse handler into IRQ12 */
void init_mouse()
{
  register_interrupt_handler(IRQ12, &mouseInput_handler);

  k_printf("MOUSE");
  //~ arrowKeyFunction("write", "left", &shiftCursor); //initialized left and right arrows to shiftCursor()
  //~ arrowKeyFunction("write", "up", &printInputBuffer); //initialized up and down arrows to shiftCursor()
  //~ arrowKeyFunction("write", "left", &notmalCursor); //initialized left and right arrows to shiftCursor()
  //~ arrowKeyFunction("write", "up", &normalVCursor); //initialized up and down arrows to shiftCursor()
  
}
