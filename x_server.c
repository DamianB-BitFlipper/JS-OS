#include "mouse.h"
#include "isr.h"
#include "x_server.h"

#include "k_math.h"
#include "k_stdio.h"
#include "k_shell.h"
#include "timer.h"

#include "graphics.h"
#include "vesa.h"

extern int VGA_width, VGA_height, globalFreq;
extern int widthVESA, heightVESA, depthVESA; //size of VESA Screen attributes

extern unsigned char *double_buffer;

extern volatile int wid;

extern volatile window_t current_window;
extern volatile window_t *window_list;

#define wVESA     1024
#define hVESA     768
#define dVESA     32

//~ static int numberOfObjects = 5;
#define numberOfObjects  5
objects arrayOfObjects[numberOfObjects]; //make array of the 5 attributes of the 4 objects


long int mousePixbuf[400] = //defines the mouse pixbuf
{
  
  0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  
  0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, -2, -2, -2, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, -2, 0x000000, -2, -2, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, -2, 0x000000, -2, 0x000000, -2, -2, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, -2, 0x000000, -2, -2, 0x000000, -2, -2, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  0x000000, 0x000000, -2, -2, -2, -2, 0x000000, -2, -2, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, 0x000000, -2, -2, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, 0x000000, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2

};

long int startMenuPixbuf[400] = //defines the mouse pixbuf
{
  
  -3, -3, -3, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C,
  
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3,
  0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3,
  -3, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, 0x83F52C, -3, -3, -3, -3, -3, -3, -3, -3, -3

};


//long int mousePixbuf[400] = //defines the mouse pixbuf
//{
  
  //0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x2e2e2e, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0xffffff, 0x000000, 0xffffff, 0xffffff, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0xffffff, 0x000000, -2, 0x000000, 0xffffff, 0xffffff, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0xffffff, 0x000000, -2, -2, 0x000000, 0xffffff, 0xffffff, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //0x000000, 0x000000, -2, -2, -2, -2, 0x000000, 0xffffff, 0xffffff, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //-2, -2, -2, -2, -2, -2, 0x000000, 0xffffff, 0xffffff, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //-2, -2, -2, -2, -2, -2, -2, 0x000000, 0x000000, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  //-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2

//};

void moveMouse(int x, int y)
{
  static int firstTime, secondTime, rep = 0, runningX = 0, runningY = 0;

  rep = (rep + 1) % 2;
  if(rep == 0)
  {
    firstTime = getSystemUpTime();
  }else if(rep == 1)
  {
    secondTime = getSystemUpTime();
  }

  if((math_abs(secondTime - firstTime) > 10 || math_abs(runningX) > 40 || math_abs(runningY) > 40) && (x != 0 || y != 0))
  //~ if((math_abs(secondTime - firstTime) > 10))
  {
  
    int xMovement = math_abs(x) + math_abs(runningX), yMovement = math_abs(y) + math_abs(runningY);
  
    translateObject(MOUSE_INDEX, x + runningX, y + runningY);
    //~ translateObject(MOUSE_INDEX, x, y);

    runningX = 0;
    runningY = 0;

  }else{
    runningX = runningX + x;
    runningY = runningY + y;
  }
}

void xLeftClick()
{
  static long long int timePassed, secondTime, timeSwitcher = 0;

  timeSwitcher = (timeSwitcher + 1) % 2;

  int index = highestWindowAbovePoint(window_list[MOUSE_INDEX].x, window_list[MOUSE_INDEX].y); //finds the top window just below the mouse
  int buttonIndex = buttonOnWindow(window_list[MOUSE_INDEX].x, window_list[MOUSE_INDEX].y, index);
  
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  if(timeSwitcher == 0)
  {
    timePassed = getSystemUpTime();
  }else if(timeSwitcher == 1)
  {
    secondTime = getSystemUpTime();
  }


  if(buttonIndex != -1 && *window_list[index].buttons[buttonIndex].onMouseLeftClick != 0 && math_abs(secondTime - timePassed) >= 75) //if function is not NULL and the time between clicks is > 100 milliseconds to prevent any acidental double clicks
  {
    window_list[index].buttons[buttonIndex].onMouseLeftClick(index);
    //~ playNote("C5", 250);
  }
}

void xMiddleClick()
{
  int index = indexOfHighestObjectAbovePoint(arrayOfObjects[1].x, arrayOfObjects[1].y, &arrayOfObjects[0], numberOfObjects);
  
  if(*arrayOfObjects[index].onMiddleClick != 0) //if function is not NULL
  {
    arrayOfObjects[index].onMiddleClick();
  }
}

void xRightClick()
{
  int index = indexOfHighestObjectAbovePoint(arrayOfObjects[1].x, arrayOfObjects[1].y, &arrayOfObjects[0], numberOfObjects);
  
  if(*arrayOfObjects[index].onRightClick != 0) //if function is not NULL
  {
    arrayOfObjects[index].onRightClick();
  }
}

void jsStartMenu()
{
  //~ putRect(0, 0, 100, 100, 5);
  static int repeat = 0;

  repeat = (repeat + 1) % 2;

  if(repeat == 1)
  {
    
    arrayOfObjects[4].name = "start menu popup"; //The Start Menu
    arrayOfObjects[4].x = 0; //The Start Menu
    arrayOfObjects[4].y = hVESA - hVESA * (1.0 / 20) - hVESA * (7.0 / 32);
    arrayOfObjects[4].width = wVESA * (1.0 / 5);
    arrayOfObjects[4].height = hVESA * (7.0 / 32);
    arrayOfObjects[4].color = 0xaaaaaa;
    
    arrayOfObjects[4].borderColor = 0x000000;
    arrayOfObjects[4].borderSize = 2;
    arrayOfObjects[4].priority = 1.0101; //first subobject of the subobject start menu
    arrayOfObjects[4].belongsToIndex = 3; //subobject belongs to index 3
    arrayOfObjects[4].shouldHide = FALSE; //sets the object to not be hidden

    putBorderOnObject(&arrayOfObjects[0], 4);

    //~ putHVLine(arrayOfObjects[4].x, arrayOfObjects[4].y, arrayOfObjects[4].width - arrayOfObjects[4].borderSize, arrayOfObjects[4].borderColor, "H");
    //~ putHVLine(arrayOfObjects[4].x, arrayOfObjects[4].y + 1, arrayOfObjects[4].width - arrayOfObjects[4].borderSize, arrayOfObjects[4].borderColor, "H");
    //~ 
    //~ putHVLine(arrayOfObjects[4].x, arrayOfObjects[4].y + arrayOfObjects[4].height - arrayOfObjects[4].borderSize, arrayOfObjects[4].width, arrayOfObjects[4].borderColor, "H");
    //~ putHVLine(arrayOfObjects[4].x, arrayOfObjects[4].y + arrayOfObjects[4].height - arrayOfObjects[4].borderSize + 1, arrayOfObjects[4].width, arrayOfObjects[4].borderColor, "H");
    //~ 
    //~ putHVLine(arrayOfObjects[4].x, arrayOfObjects[4].y, arrayOfObjects[4].height, arrayOfObjects[4].borderColor, "V");
    //~ putHVLine(arrayOfObjects[4].x+1, arrayOfObjects[4].y, arrayOfObjects[4].height, arrayOfObjects[4].borderColor, "V");
    //~ 
    //~ putHVLine(arrayOfObjects[4].x + arrayOfObjects[4].width - arrayOfObjects[4].borderSize, arrayOfObjects[4].y, arrayOfObjects[4].height, arrayOfObjects[4].borderColor, "V");
    //~ putHVLine(arrayOfObjects[4].x+1 + arrayOfObjects[4].width - arrayOfObjects[4].borderSize, arrayOfObjects[4].y, arrayOfObjects[4].height, arrayOfObjects[4].borderColor, "V");
    
    putRect(arrayOfObjects[4].x + arrayOfObjects[4].borderSize, arrayOfObjects[4].y + arrayOfObjects[4].borderSize, arrayOfObjects[4].width - 2 * arrayOfObjects[4].borderSize, arrayOfObjects[4].height - 2 * arrayOfObjects[4].borderSize, arrayOfObjects[4].color);
  }else if(repeat == 0)
  {
    arrayOfObjects[4].shouldHide = TRUE; //sets the object to not be hidden
    refreshObjects(&arrayOfObjects[0], numberOfObjects, 4, 0, 0, 0);
  }
}


int fpsTEST()
{
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  int up, down, count = 0;

  up = getSystemUpTime();

  while(1)
  {
    refreshScreen_VESA();
    down = getSystemUpTime();
    if(down - up > 1000)
    {
      break;
    }else{
      count++;
    }
  }

  return count;
}


void xServer()
{
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  VGA_init(1024, 768, 24);
  init_window_manager();

  //~ createWindow("POOP", 0, 0, 1, 1024, 768, 0);
  createWindow("TEST", 20, 20, 1, 200, 200, 0);
  createObject("JSPANEL", 0, hVESA - 20, 2, wVESA, 20, 0xFFC100, 0);
  addButtonToObject(0, 0, 20, 20, startMenuPixbuf, &destroyWindow, 2);

  createWindow("JSPANEL", 200, 200, 3, 500, 500, 0xFFC100, 0);
  
  createPixbufObject("MOUSE", mousePixbuf, 400, 400, MOUSE_INDEX, 20, 20, 0);

  mouseClickFunctions("write", "left", &xLeftClick);
  mouseClickFunctions("write", "middle", &xMiddleClick);
  mouseClickFunctions("write", "right", &xRightClick);
  
  //~ int a;
  //~ for

  refresh_BufScreen();

  //int count = fpsTEST(), f;

  //for(f = 0; f < count; f++)
  //{
    //playNote("C5", 250);
    //playNote("C6", 250);
  //}
  
  //~ createPixbufObject(
  //~ kmalloc(120000);
}
