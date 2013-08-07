/*
 * x_server.c
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

extern int VGA_width, VGA_height;
extern int widthVESA, heightVESA, depthVESA; //size of VESA Screen attributes

extern unsigned char *double_buffer;

extern volatile int wid;

extern volatile window_t current_window;
extern volatile window_t *window_list;

#define wVESA     1024
#define hVESA     768
#define dVESA     32

#define numberOfObjects  5
objects arrayOfObjects[numberOfObjects]; //make array of the 5 attributes of the 4 objects

///*WIDGETS*/
window_t mouse, js_panel, desktop;
///*WIDGETS*/

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

void moveMouse(int x, int y)
{
  static int firstTime, secondTime, rep = 0, runningX = 0, runningY = 0;

  rep = (rep + 1) % 2;

  if(!rep)
    firstTime = getSystemUpTime();
  else if(rep)
    secondTime = getSystemUpTime();

  if((math_abs(secondTime - firstTime) > 10 || math_abs(runningX) > 40 || 
      math_abs(runningY) > 40) && (x != 0 || y != 0))
  {
  
    int xMovement = math_abs(x) + math_abs(runningX), yMovement = math_abs(y) + math_abs(runningY);
  
    translateObject(mouse.id, x + runningX, y + runningY);

    runningX = 0;
    runningY = 0;

  }else{
    runningX = runningX + x;
    runningY = runningY + y;
  }
}

void xLeftClick() //mouse left click call back
{
  static long long int timePassed, secondTime, timeSwitcher = 0;

  timeSwitcher = (timeSwitcher + 1) % 2;

  int index = highestWindowAbovePoint(window_list[mouse.id].x, window_list[mouse.id].y, mouse.id); //finds the top window just below the mouse
  int buttonIndex = buttonOnWindow(window_list[mouse.id].x, window_list[mouse.id].y, index);
  
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  if(!timeSwitcher)
    timePassed = getSystemUpTime();
  else if(timeSwitcher)
    secondTime = getSystemUpTime();

  //if function is not NULL and the time between clicks is > 100 milliseconds to prevent any acidental double clicks
  if(buttonIndex != -1 && *window_list[index].buttons[buttonIndex].onMouseLeftClick != 0 && 
     math_abs(secondTime - timePassed) >= 75) 
    window_list[index].buttons[buttonIndex].onMouseLeftClick(window_list[index]);

}

void xMiddleClick() //mouse middle click call back
{
  int index = indexOfHighestObjectAbovePoint(arrayOfObjects[1].x, arrayOfObjects[1].y, &arrayOfObjects[0], numberOfObjects);
  
  if(*(arrayOfObjects[index].onMiddleClick)) //if function is not NULL
    arrayOfObjects[index].onMiddleClick();
}

void xRightClick() //mouse right click call back
{
  int index = indexOfHighestObjectAbovePoint(arrayOfObjects[1].x, arrayOfObjects[1].y, &arrayOfObjects[0], numberOfObjects);
  
  if(*(arrayOfObjects[index].onRightClick)) //if function is not NULL
    arrayOfObjects[index].onRightClick();
}

//TODO make this work
void jsViewer()
{
  window_t jsViewerWindow;

  jsViewerWindow = createWindow("JSVIEWER", 0, 0, 0, 350, 450, desktop);

  currentWindow(jsViewerWindow);
}

void jsStartMenu()
{
  static int repeat = 0;
  static window_t js_panelStartMenu;

  repeat = (repeat + 1) % 2;

  if(repeat == ON)
  {
    js_panelStartMenu = createObject("START_Menu", 0, hVESA - 20 - 75, 1, 75, 75, 0xD5D5D5, js_panel);
    addButtonToObject(0, 0, 20, 20, startMenuPixbuf, &jsViewer, js_panelStartMenu);
  
  }else if(repeat == OFF)
    destroyWindow(js_panelStartMenu);
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

  window_t firstWindow, secondWindow, thirdWindow;

  VGA_init(1024, 768, 24);
  init_window_manager();

  asm volatile("cli"); //disables intterpts so mouse does not do anything funky while drawing starting windows

  firstWindow = createWindow("TEST", 20, 20, 1, 200, 200, desktop);
  js_panel = createObject("JSPANEL", 0, hVESA - 20, 2, wVESA, 20, 0xFFC100, desktop);
  addButtonToObject(0, 0, 20, 20, startMenuPixbuf, &jsStartMenu, js_panel);

  secondWindow = createWindow("TEST", 200, 200, 3, 500, 500, desktop);

  
  mouse = createPixbufObject("MOUSE", mousePixbuf, wVESA / 2, hVESA / 2, MAX_PRIORITY, 20, 20, desktop);

  thirdWindow = createWindow("TEST", 300, 100, 4, 500, 500, desktop);

  currentWindow(secondWindow);
  
  mouseClickFunctions("write", "left", &xLeftClick); //sets call backs for mouse
  mouseClickFunctions("write", "middle", &xMiddleClick);
  mouseClickFunctions("write", "right", &xRightClick);
  
  refresh_BufScreen();

  asm volatile("sti"); //reenables those interupts

}
