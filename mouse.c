#include "mouse.h"
#include "isr.h"
#include "x_server.h"

#include "k_math.h"
#include "k_stdio.h"
#include "k_shell.h"

#include "graphics.h"


//Mouse.inc by SANiK
//License: Use as you wish, except to cause damage

typedef unsigned char byte; 
typedef signed char sbyte; 
typedef unsigned int dword;

extern VGA_width, VGA_height;

//~ byte mouse_cycle = 0;   //unsigned char
//~ sbyte mouse_byte[3];    //signed char
//~ sbyte mouse_x = 0;      //signed char
//~ sbyte mouse_y = 0;      //signed char
//~ int mouse_x = 0;        //signed char
//~ int mouse_y = 0;        //signed char

//static int numberOfObjects = 2;
//objects arrayOfObjects[2]; //make array of the 5 attributes of the 4 objects

//int mouseWidth = 10, mouseHeight = 10;
////~ int mousePixbuf[10][10];

//int mousePixbuf[10][10] = //defines the mouse pixbuf
////~ int mousePixbuf[9][9] = //defines the mouse pixbuf
//{
  //{LIGHT_GREEN, 15, 15, 15, 15, 15, 15, 15, 15, 15},
  //{LIGHT_GREEN, LIGHT_GREEN, 15, 15, 15, 15, 15, 15, 15, 15},
  //{LIGHT_GREEN, 15, LIGHT_GREEN, 15, 15, 15, 15, 15, 15, 15},
  //{LIGHT_GREEN, 15, 15, LIGHT_GREEN, 15, 15, 15, 15, 15, 15},
  //{LIGHT_GREEN, 15, 15, 15, LIGHT_GREEN, 15, 15, 15, 15, 15},
  //{LIGHT_GREEN, 15, 15, 15, 15, LIGHT_GREEN, 15, 15, 15, 15},
  ////~ {LIGHT_GREEN, 15, 15, 15, 15, 15, LIGHT_GREEN, 15, 15, 15},
  //{LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, 15, 15, 15},
  //{15, 15, 15, LIGHT_GREEN, 15, 15, 15, 15, 15, 15},
  //{15, 15, 15, LIGHT_GREEN, 15, 15, 15, 15, 15, 15},
  //{15, 15, 15, LIGHT_GREEN, 15, 15, 15, 15, 15, 15}

//};

//void moveMouse(int x, int y)
//{
  //int xMovement = math_abs(x), yMovement = math_abs(y);

  //int prevX = arrayOfObjects[1].x, prevY = arrayOfObjects[1].y;
  
  //arrayOfObjects[1].x = arrayOfObjects[1].x + x;

  //if(arrayOfObjects[1].x > VGA_width)
  //{
    //arrayOfObjects[1].x = VGA_width;
  //}else if(arrayOfObjects[1].x < 0)
  //{
    //arrayOfObjects[1].x = 0;
  //}
  
  //arrayOfObjects[1].y = arrayOfObjects[1].y - y;
  
  //if(arrayOfObjects[1].y > VGA_height)
  //{
    //arrayOfObjects[1].y = VGA_height;
  //}else if(arrayOfObjects[1].y < 0)
  //{
    //arrayOfObjects[1].y = 0;
  //}
  
  //refreshObjects(&arrayOfObjects[0], numberOfObjects, 1, xMovement + 2, yMovement + 2, &mousePixbuf[0][0]);
  ////~ refreshArea(&arrayOfObjects[0], numberOfObjects, arrayOfObjects[1].x, arrayOfObjects[1].y, arrayOfObjects[1].width, arrayOfObjects[1].height);
  ////~ refreshArea(&arrayOfObjects[0], numberOfObjects, prevX, prevY, arrayOfObjects[1].width, arrayOfObjects[1].height);
//}

//void startMouseEnvironment()
//{
  ///*initialize the objects attributes*/
  //arrayOfObjects[0].x = 0; //Background
  //arrayOfObjects[0].y = 0;
  ////~ arrayOfObjects[0].width = VGA_width;
  //arrayOfObjects[0].width = 320;
  ////~ arrayOfObjects[0].height = VGA_height;
  //arrayOfObjects[0].height = 200;
  //arrayOfObjects[0].color = WHITE;
  //arrayOfObjects[0].priority = 0;

  //arrayOfObjects[1].x = 160; //The mouse
  //arrayOfObjects[1].y = 100;
  ////~ arrayOfObjects[1].width = 5;
  //arrayOfObjects[1].width = 10;
  ////~ arrayOfObjects[1].height = 30;
  //arrayOfObjects[1].height = 10;
  //arrayOfObjects[1].color = -1;
  ////~ arrayOfObjects[1].priority = 2;
  //arrayOfObjects[1].priority = 1;

  //VGA_init(320, 200, 256); //initialize the gui

  //putRect(arrayOfObjects[0].x, arrayOfObjects[0].y, arrayOfObjects[0].width, arrayOfObjects[0].height, arrayOfObjects[0].color);
  ////~ putRect(arrayOfObjects[1].x, arrayOfObjects[1].y, arrayOfObjects[1].width, arrayOfObjects[1].height, arrayOfObjects[1].color);  
  //putPixbufRect(arrayOfObjects[1].x, arrayOfObjects[1].y, arrayOfObjects[1].width, arrayOfObjects[1].height, &mousePixbuf[0][0]);  

  ////~ refreshScreen(&arrayOfObjects[0], numberOfObjects);

//}

void mouseClickFunctions(char *callOrWrite, char *buttonClick, void (*func)() )
{
static void (*leftClick)(void);
static void (*middleClick)(void);
static void (*rightClick)(void);
  
  if(k_strcmp(buttonClick, "left") == 0 && k_strcmp(callOrWrite, "write") == 0) //sets input function for left button click
  {
    leftClick = func;

  }else if(k_strcmp(buttonClick, "middle") == 0 && k_strcmp(callOrWrite, "write") == 0) //sets input function for middle button click
  {
    middleClick = func;

  }else if(k_strcmp(buttonClick, "right") == 0 && k_strcmp(callOrWrite, "write") == 0) //sets input function for right button click
  {
    rightClick = func;

  }

  if(k_strcmp(buttonClick, "left") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls left button clicked
  {
    (*leftClick)();
  }else if(k_strcmp(buttonClick, "middle") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls middle button clicked
  {
    (*middleClick)();
  }else if(k_strcmp(buttonClick, "right") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls right button clicked
  {
    (*rightClick)();
  }



}

void disableMouseButtons() //does nothing so when written to mouseClickFunctions() for a specific button, it will do nothing
{

}

//Mouse functions
void mouse_handler()
{
  static unsigned char cycle = 0;
  static char mouse_bytes[3];

  static int x = 0, y = 0, signX = 0, signY = 0;
  
  mouse_bytes[cycle++] = inb(0x60);
 
  if (cycle == 3)
  { // if we have all the 3 bytes...
    cycle = 0; // reset the counter
    // do what you wish with the bytes, this is just a sample
    if ((mouse_bytes[0] & 0x80) || (mouse_bytes[0] & 0x40))
    {
      return; // the mouse only sends information about overflowing, do not care about it and return
    }
    
    //if (!(mouse_bytes[0] & 0x20))
    //{
      ////~ k_printf("Y\n");
      ////~ mouse_y |= 0xFFFFFF00; //delta-y is a negative value
      //y |= 0xFFFFFF00; //delta-y is a negative value

      ////~ k_putChar(mouse_y);
    //}
    
    //if (!(mouse_bytes[0] & 0x10))
    //{
      ////~ k_printf("X\n");
      ////~ mouse_x |= 0xFFFFFF00; //delta-x is a negative value
      //x |= 0xFFFFFF00; //delta-x is a negative value

      //k_printf("%d\n", x);
      ////~ k_putChar(mouse_x);
      
    //}
    
    if (mouse_bytes[0] & 0x4) //Middle button pressed
    {
      //~ k_printf("Middle button is pressed!\n");
      mouseClickFunctions("call", "middle", &disableMouseButtons);
    }
    
    if (mouse_bytes[0] & 0x2) //Right button pressed
    {
      //~ k_printf("Right button is pressed!\n");
      mouseClickFunctions("call", "right", &disableMouseButtons);
      
    }
    
    if (mouse_bytes[0] & 0x1) //Left button pressed
    {
      //~ k_printf("Left button is pressed!\n");
      mouseClickFunctions("call", "left", &disableMouseButtons);
      
    }

    //~ mouse_x = mouse_bytes[1];
    //~ mouse_y = mouse_bytes[2];
    x = mouse_bytes[1]; //amout mouse moves in y axis
    y = mouse_bytes[2]; //amout mouse moves in x axis
    //~ signX = mouse_bytes[0] & 0x10;
    //~ signY = mouse_bytes[0] & 0x10;

    //if(signX == 0) //delta-x is positive
    //{

    //}else if(signX == 16) //delta-x is negative
    //{
      //x = math_abs(x);
      //k_printf("%d\t%d\n", x, signX);

    //}

    moveMouse(x, y);

    //~ k_printf("%d\t%d\n", x, signX);
    //~ k_printf("%s\n", mouse_x);
    
    // do what you want here, just replace the puts's to execute an action for each button
    // to use the coordinate data, use mouse_bytes[1] for delta-x, and mouse_bytes[2] for delta-y
  }
}


inline void mouse_wait(byte a_type) //unsigned char
{
  dword _time_out=100000; //unsigned int
  if(a_type==0)
  {
    while(_time_out--) //Data
    {
      if((inb(0x64) & 1)==1)
      {
        return;
      }
    }
    return;
  }
  else
  {
    while(_time_out--) //Signal
    {
      if((inb(0x64) & 2)==0)
      {
        return;
      }
    }
    return;
  }
}

inline void mouse_write(byte a_write) //unsigned char
{
  //Wait to be able to send a command
  mouse_wait(1);
  //Tell the mouse we are sending a command
  outb(0x64, 0xD4);
  //Wait for the final part
  mouse_wait(1);
  //Finally write
  outb(0x60, a_write);
}

byte mouse_read()
{
  //Get's response from mouse
  mouse_wait(0);
  return inb(0x60);
}

void init_mouse()
{
  byte _status;  //unsigned char

  //Enable the auxiliary mouse device
  mouse_wait(1);
  outb(0x64, 0xA8);
 
  //Enable the interrupts
  mouse_wait(1);
  outb(0x64, 0x20);
  mouse_wait(0);
  _status=(inb(0x60) | 2);
  mouse_wait(1);
  outb(0x64, 0x60);
  mouse_wait(1);
  outb(0x60, _status);
 
  //Tell the mouse to use default settings
  mouse_write(0xF6);
  mouse_read();  //Acknowledge
 
  //Enable the mouse
  mouse_write(0xF4);
  mouse_read();  //Acknowledge

  //Setup the mouse handler
  register_interrupt_handler(IRQ12, &mouse_handler);

  /*writes functionability to mouse buttons*/
  mouseClickFunctions("write", "left", &disableMouseButtons); //left button
  mouseClickFunctions("write", "middle", &disableMouseButtons); //middle button
  mouseClickFunctions("write", "right", &disableMouseButtons); //right button
  

  //TEST, begins the gui desktop environment with mouse capabilities
  //~ xServer();
}

/* Handles the mouse interrupt */
void mouseInput_handler()
{

  //~ k_printf("\tmoved");
  unsigned char scancode;
  //~ int speciaKeyNumber;

  /* Read from the mouse's data buffer */
  scancode = inb(0x64);

  k_putChar(scancode);
  k_printf("\n");
  
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
void mouse_install()
{
  register_interrupt_handler(IRQ12, &mouseInput_handler);

  k_printf("MOUSE");
  //~ arrowKeyFunction("write", "left", &shiftCursor); //initialized left and right arrows to shiftCursor()
  //~ arrowKeyFunction("write", "up", &printInputBuffer); //initialized up and down arrows to shiftCursor()
  //~ arrowKeyFunction("write", "left", &notmalCursor); //initialized left and right arrows to shiftCursor()
  //~ arrowKeyFunction("write", "up", &normalVCursor); //initialized up and down arrows to shiftCursor()
  
}
