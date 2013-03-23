#include "mouse.h"
#include "isr.h"
#include "x_server.h"

#include "k_math.h"
#include "k_stdio.h"
#include "k_shell.h"
//~ #include "timer.h"

#include "graphics.h"

//~ extern int globalFreq;


typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned int dword;

extern VGA_width, VGA_height;

int leftButtonPressed = FALSE, rightButtonPressed = FALSE, middleButtonPressed = FALSE;

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

  static int x = 0, y = 0, prevRight = 0, prevLeft = 0, prevMiddle = 0;

  mouse_bytes[cycle++] = inb(0x60);

  if (cycle == 3)
  { // if we have all the 3 bytes...
    cycle = 0; // reset the counter
    // do what you wish with the bytes, this is just a sample
    if ((mouse_bytes[0] & 0x80) || (mouse_bytes[0] & 0x40))
    {
      return; // the mouse only sends information about overflowing, do not care about it and return
    }

    ///*BUTTON PRESS*/

    if ((mouse_bytes[0] & 0x4) && prevMiddle == 0) //Middle button pressed and previously, not buttons have been pressed down
    {
      middleButtonPressed = TRUE;
      
      //~ k_printf("Middle button is pressed!\n");
      mouseClickFunctions("call", "middle", &disableMouseButtons);

    }

    if ((mouse_bytes[0] & 0x2) && prevRight == 0) //Right button pressed and previously, not buttons have been pressed down
    {
      rightButtonPressed = TRUE;
      
      //~ k_printf("Right button is pressed!\n");
      mouseClickFunctions("call", "right", &disableMouseButtons);

    }

    if ((mouse_bytes[0] & 0x1) && prevLeft == 0) //Left button pressed and previously, not buttons have been pressed down
    {
      leftButtonPressed = TRUE;
      
      //~ k_printf("Left button is pressed!\n");
      mouseClickFunctions("call", "left", &disableMouseButtons);

    }

    ///*BUTTON RELEASE*/

    if ((mouse_bytes[0] & 0x4) == 0 && prevMiddle == 4) //Middle button released
    {
      middleButtonPressed = FALSE;

      //~ k_printf("Middle button is released!\n");
      //~ mouseClickFunctions("call", "middle", &disableMouseButtons);

    }

    if ((mouse_bytes[0] & 0x2) == 0 && prevRight == 2) //Right button release
    {
      rightButtonPressed = FALSE;
      
      //~ k_printf("Right button is released!\n");
      //~ mouseClickFunctions("call", "right", &disableMouseButtons);

    }

    if ((mouse_bytes[0] & 0x1) == 0 && prevLeft == 1) //Left button released
    {
      leftButtonPressed = FALSE;
      
      //~ k_printf("Left button is released!\n");
      //~ mouseClickFunctions("call", "left", &disableMouseButtons);

    }

    //~ mouse_x = mouse_bytes[1];
    //~ mouse_y = mouse_bytes[2];
    x = mouse_bytes[1]; //amout mouse moves in y axis
    y = mouse_bytes[2]; //amout mouse moves in x axis

    moveMouse(x, y);

    //~ k_printf("%d\t%d\n", x, signX);

    //~ k_printf("\n%h\n", mouse_bytes[3] & 0x2);

    prevMiddle = mouse_bytes[0] & 4;
    prevRight = mouse_bytes[0] & 2;
    prevLeft = mouse_bytes[0] & 1;

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

  //Disable the mouse
  //~ mouse_write(0xF5);
  //~ byte mouseID = mouse_read();  //Acknowledge
//~
  //~ k_printf("\n\nMouse id is: %h", mouseID);

  ///*MAKE SCROLL WHEEL WORK*/
  //TODO make scroll wheel work

  //mouse_write(0xF3);
  //mouse_write(100);

  ////~ mouse_write(0xF3);
  ////~ mouse_write(0x64);
  ////~
  ////~ mouse_write(0xF3);
  ////~ mouse_write(0x50);

  ////~ asm volatile("sti");
  ////~ init_timer(globalFreq); // Initialise timer to globalFreq-Hz
  //mouse_wait(1);

  ////~ mouse_write(0xE9);

  ////~ mouse_wait(1);
  ////~ mSleep(10);

  //mouse_read();  //Acknowledge
  //mouse_read();  //Acknowledge
  //mouse_read();  //Acknowledge

  //byte mouseID = mouse_read();  //Acknowledge

  //k_printf("\n\nMouse id is: %h", mouseID);

  ///*MAKE SCROLL WHEEL WORK*/

  //Setup the mouse handler
  register_interrupt_handler(IRQ12, &mouse_handler);

  /*writes functionability to mouse buttons*/
  mouseClickFunctions("write", "left", &disableMouseButtons); //left button
  mouseClickFunctions("write", "middle", &disableMouseButtons); //middle button
  mouseClickFunctions("write", "right", &disableMouseButtons); //right button


  //TEST, begins the gui desktop environment with mouse capabilities
  //~ xServer();
}

void disableMousePackets()
{
  mouse_write(0xF5);
  mouse_read();  //Acknowledge

}

void enableMousePackets()
{
  mouse_write(0xF4);
  mouse_read();  //Acknowledge
}
///* Handles the mouse interrupt */
//void mouseInput_handler()
//{

  ////~ k_printf("\tmoved");
  //unsigned char scancode;
  ////~ int speciaKeyNumber;

  ///* Read from the mouse's data buffer */
  //scancode = inb(0x64);

  //k_putChar(scancode);
  //k_printf("\n");


//}

///* Installs the mouse handler into IRQ12 */
//void mouse_install()
//{
  //register_interrupt_handler(IRQ12, &mouseInput_handler);

  //k_printf("MOUSE");
  ////~ arrowKeyFunction("write", "left", &shiftCursor); //initialized left and right arrows to shiftCursor()
  ////~ arrowKeyFunction("write", "up", &printInputBuffer); //initialized up and down arrows to shiftCursor()
  ////~ arrowKeyFunction("write", "left", &notmalCursor); //initialized left and right arrows to shiftCursor()
  ////~ arrowKeyFunction("write", "up", &normalVCursor); //initialized up and down arrows to shiftCursor()

//}
