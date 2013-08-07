/*
 * vesa.c
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

extern window_t desktop;

extern s32int middleButtonPressed, rightButtonPressed, leftButtonPressed;

#define wVESA     1024
#define hVESA     768
#define dVESA     32

// define our structure
typedef struct __attribute__ ((packed)) {
  unsigned short di, si, bp, sp, bx, dx, cx, ax;
  unsigned short gs, fs, es, ds, eflags;
} regs16_t;

int widthVESA, heightVESA, depthVESA;

unsigned char *vga_mem; //pointer where we assign our vga address

/*Typedef for VESA mode information*/
typedef struct MODE_INFO
{
  unsigned short ModeAttributes       __attribute__ ((packed));
  unsigned char  WinAAttributes;//       __attribute__ ((packed));
  unsigned char  WinBAttributes;//       __attribute__ ((packed));
  unsigned short WinGranularity       __attribute__ ((packed));
  unsigned short WinSize              __attribute__ ((packed));
  unsigned short WinASegment          __attribute__ ((packed));
  unsigned short WinBSegment          __attribute__ ((packed));
  unsigned long  WinFuncPtr           __attribute__ ((packed));
  unsigned short BytesPerScanLine     __attribute__ ((packed));
  unsigned short XResolution          __attribute__ ((packed));
  unsigned short YResolution          __attribute__ ((packed));
  unsigned char  XCharSize;//            __attribute__ ((packed));
  unsigned char  YCharSize;//            __attribute__ ((packed));
  unsigned char  NumberOfPlanes;//       __attribute__ ((packed));
  unsigned char  BitsPerPixel;//         __attribute__ ((packed));
  unsigned char  NumberOfBanks;//        __attribute__ ((packed));
  unsigned char  MemoryModel;//          __attribute__ ((packed));
  unsigned char  BankSize;//             __attribute__ ((packed));
  unsigned char  NumberOfImagePages;//   __attribute__ ((packed));
  unsigned char  Reserved_page;//        __attribute__ ((packed));
  unsigned char  RedMaskSize;//          __attribute__ ((packed));
  unsigned char  RedMaskPos;//           __attribute__ ((packed));
  unsigned char  GreenMaskSize;//        __attribute__ ((packed));
  unsigned char  GreenMaskPos;//         __attribute__ ((packed));
  unsigned char  BlueMaskSize;//         __attribute__ ((packed));
  unsigned char  BlueMaskPos;//          __attribute__ ((packed));
  unsigned char  ReservedMaskSize;//     __attribute__ ((packed));
  unsigned char  ReservedMaskPos;//      __attribute__ ((packed));
  unsigned char  DirectColorModeInfo;//  __attribute__ ((packed));
  unsigned long  PhysBasePtr          __attribute__ ((packed));
  unsigned long  OffScreenMemOffset   __attribute__ ((packed));
  unsigned short OffScreenMemSize     __attribute__ ((packed));
  unsigned char  Reserved[206];//        __attribute__ ((packed));
} MODE_INFO;

/*Typedef for VESA information*/
typedef struct VESA_INFO
{
  unsigned char  VESASignature[4];//     __attribute__ ((packed));
  unsigned short VESAVersion          __attribute__ ((packed));
  unsigned long  OEMStringPtr         __attribute__ ((packed));
  unsigned char  Capabilities[4];//      __attribute__ ((packed));
  unsigned long  VideoModePtr         __attribute__ ((packed));
  unsigned short TotalMemory          __attribute__ ((packed));
  unsigned short OemSoftwareRev       __attribute__ ((packed));
  unsigned long  OemVendorNamePtr     __attribute__ ((packed));
  unsigned long  OemProductNamePtr    __attribute__ ((packed));
  unsigned long  OemProductRevPtr     __attribute__ ((packed));
  unsigned char  Reserved[222];//        __attribute__ ((packed));
  unsigned char  OemData[256];//         __attribute__ ((packed));
} VESA_INFO;


//int32 runs a bios interrupt, located at v86.asm
extern void int32(u8int intnum, regs16_t *regs);

/*Sets the bank if the Linear Frame Buffer is not supported or enabled*/
void setBank(int bankNo)
{
  regs16_t regs;

  regs.ax = 0x4f05;
  regs.bx = 0x0;
  regs.dx = bankNo;

  int32(0x10, &regs);

}

//sets up VESA for mode
void setVesa(u32int mode)
{
  //stop any task switching, but do not stop interupts
  current_task->ready_to_run = FALSE;

  VESA_INFO info; //VESA information
  MODE_INFO vbeModeInfo; //VESA mode information

  regs16_t regs;

  /**Gets VESA information**/

  u32int buffer = (u32int)kmalloc(sizeof(VESA_INFO)) & 0xFFFFF; //sets the address for the buffer

  memcpy(buffer, "VBE2", 4);
  memset(&regs, 0, sizeof(regs)); //clears the registers typedef struct

  regs.ax = 0x4f00; //mode that gets VESA information
  regs.di = buffer & 0xF;
  regs.es = (buffer>>4) & 0xFFFF;
  int32(0x10, &regs); //calls v86 interupt
  memcpy(&info, buffer, sizeof(VESA_INFO)); //copies info from the buffer to the info typedef struct

  //print VESA information
  //~ k_printf("\n\nVesa Signature: %s\n", info.VESASignature);
  //~ k_printf("\n\nVesa Version: %h\n", info.VESAVersion);
  //~ k_printf("\n\nVesa Video Modes: %h\n", info.VideoModePtr);

  /**Gests VESA mode information**/

  //allocates memory for the buffer that stores the MODE_INFO for the VESA mode
  u32int modeBuffer = (u32int)kmalloc(sizeof(MODE_INFO)) & 0xFFFFF;

  memset(&regs, 0, sizeof(regs)); //clears the registers typedef struct

  regs.ax = 0x4f01; //mode the gets the VESA mode information
  regs.di = modeBuffer & 0xF;
  regs.es = (modeBuffer>>4) & 0xFFFF;
  regs.cx = mode; //mode to get the information for
  int32(0x10, &regs);
  memcpy(&vbeModeInfo, modeBuffer, sizeof(MODE_INFO));

  widthVESA = vbeModeInfo.XResolution;
  heightVESA = vbeModeInfo.YResolution;
  depthVESA = vbeModeInfo.BitsPerPixel;

  //print VESA mode information
  //~ k_printf("\nBase Pointer: %h\n", (u32int)vbeModeInfo.PhysBasePtr);
  //~ k_printf("\nXRes: %d\n", (u32int)vbeModeInfo.XResolution);
  //~ k_printf("\nYRes: %d\n", (u32int)vbeModeInfo.YResolution);
  //~ k_printf("\nBits per pixel: %d\n", (u32int)vbeModeInfo.BitsPerPixel);
  //~ k_printf("\nExits status: %h\n", (u32int)regs.ax);

  /*Sets the Linear Frame Buffer address tp vga_mem and lfb variables*/
  vga_mem = (u8int*)vbeModeInfo.PhysBasePtr;
  u32int lfb = (u32int)vbeModeInfo.PhysBasePtr;

  /**Sets up the VESA mode**/
  regs.ax = 0x4f02; //mode the sets up VESA graphics

  /*sets up mode with a linear frame buffer, the logical or (| 0x4000) tells
   * VESA VBE that we want to use a linear frame buffer*/
  regs.bx = (mode | 0x4000);
  int32(0x10, &regs);

  //reenable the interupts and the tasks to run
  asm volatile("sti");
  current_task->ready_to_run = TRUE;

}


/****DRAWING FUNCTIONS****/

#define MAX_WINDOWS 256

volatile window_t current_window;
volatile window_t *window_list;

volatile int wid = 0;

///*Buttons*/
long int closeButton[81] = //defines the mouse pixbuf
{
  0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400,
  -3, 0x007400, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800,
  -3, 0x007400, 0x00b800, 0x00b800, 0x00b800, 0xe57c00, 0x00b800, 0xe57c00, 0x00b800,
  -3, -3, 0x007400, 0x00b800, 0x00b800, 0x00b800, 0xe57c00, 0x00b800, 0x00b800,
  -3, -3, 0x007400, 0x00b800, 0x00b800, 0xe57c00, 0x00b800, 0xe57c00, 0x00b800,
  -3, -3, -3, 0x007400, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800,
  -3, -3, -3, 0x007400, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800,
  -3, -3, -3, -3, 0x007400, 0x00b800, 0x00b800, 0x00b800, 0x00b800,
  -3, -3, -3, -3, -3, 0x007400, 0x007400, 0x007400, 0x007400
};

long int maximizeButton[81] = //defines the mouse pixbuf
{
  0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800,
  0x00b800, 0x00b800, 0xe57c00, 0xe57c00, 0xe57c00, 0xe57c00, 0xe57c00, 0xe57c00, 0x00b800,
  0x00b800, 0x00b800, 0xe57c00, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0xe57c00, 0x00b800,
  0x00b800, 0x00b800, 0xe57c00, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0xe57c00, 0x00b800,
  0x00b800, 0x00b800, 0xe57c00, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0xe57c00, 0x00b800,
  0x00b800, 0x00b800, 0xe57c00, 0xe57c00, 0xe57c00, 0xe57c00, 0xe57c00, 0xe57c00, 0x00b800,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800,
  0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400
};

long int minimizeButton[81] = //defines the mouse pixbuf
{
  0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400, 0x007400,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x007400, -3,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x007400, -3,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x007400, -3, -3,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x007400, -3, -3,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x007400, -3, -3, -3,
  0x00b800, 0xe57c00, 0xe57c00, 0xe57c00, 0x00b800, 0x007400, -3, -3, -3,
  0x00b800, 0x00b800, 0x00b800, 0x00b800, 0x007400, -3, -3, -3, -3,
  0x007400, 0x007400, 0x007400, 0x007400, -3, -3, -3, -3, -3
};

void init_window_manager()
{
  //We need to initialise the Desktop
  static window_t window;
  window.name = "Desktop";
  window.x = 0;
  window.y = 0;
  window.z = 0;
  window.width = wVESA;
  window.height = hVESA;
  window.parentid = 0;
  window.id = 0;
  
  window.data = (u32int*)kmalloc((wVESA * hVESA) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0xff, (wVESA * hVESA) * (dVESA / 8));

  //~ k_printf("\n background data address: %h", window.data);
  write_buffer(0, 0, 1024, 768, (u32int*)window.data);
  refreshScreen_VESA();

  current_window = window;

  window_list = (window_t*)kmalloc(sizeof(window_t) * MAX_WINDOWS);
  window_list[window.id] = current_window;

  desktop = window;
}

void updateWindowListData(window_t object)
{
  int objectID = object.id;
  
  window_list[objectID].data = object.data;
  window_list[objectID].name = object.name;
  window_list[objectID].id = object.id;
  window_list[objectID].x = object.x;
  window_list[objectID].y = object.y;
  window_list[objectID].z = object.z;
  window_list[objectID].width = object.width;
  window_list[objectID].height = object.height;
  window_list[objectID].parentid = object.parentid;

}

void currentWindow(window_t object)
{
  int rep;
  int windowWithHighestPriority = 0;

  for(rep = 0; rep < wid + 1; rep++)
    if(window_list[rep].z < MAX_PRIORITY && window_list[rep].z > windowWithHighestPriority)
      windowWithHighestPriority = window_list[rep].z;

  object.z = windowWithHighestPriority + 1;
  updateWindowListData(object);

  refresh_BufObject(object, 1, 1);

}

/*gets the new id for a window*/
static int getnewwid()
{
  if(wid != MAX_WINDOWS - 1)
  {
    wid++;
    return wid;
  }else{
    int run;

    for(run = 0; run < MAX_WINDOWS; run++)
      if(*window_list[run].data == 0)
      {
        break;
        return run;
      }
  }
}

void addButtonToObject(int x, int y, int width, int height, long int *pixBuf, void(*call_back)(window_t widget), window_t parent)
{
  int buttonNumber, tmp, assigned = FALSE;

  for(tmp = 0; tmp < MAX_BUTTONS; tmp++)
  {
    if(window_list[parent.id].buttons[tmp].onMouseLeftClick == 0) //gets the button id of an unused button space
    {
      buttonNumber = tmp;
      assigned = TRUE;
      break;
    }
  }

  if(assigned == TRUE)
  {
    window_list[parent.id].buttons[buttonNumber] = createButton(x, y, width, height, pixBuf, call_back, window_list[parent.id]);
    refresh_BufArea(window_list[parent.id].x + x, window_list[parent.id].y + y, width, height, TRUE);
  }
}

/*creates a object with a constant fill*/
window_t createObject(char *windowname, int x, int y, int priority, int width, int height, int fill, window_t parent)
{
  static window_t window;
  window.name = windowname;
  window.x = x;
  window.y = y;

  if(parent.id != desktop.id) //if the parent is not the desktop, ie:user wants to create a new object not a sub object
  {
    window.z = parent.z + ((float)priority / MAX_CHILDREN);
  }else{
    window.z = priority;
  }

  window.width = width;
  window.height = height;
  window.parentid = parent.id;
  window.id = getnewwid();
  window.data = (u32int*)kmalloc((width * height) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0xff, (width * height) * (dVESA / 8));

  ///* Fill */
  plot_BufRect(0, 0, width, height, FALSE, 0, fill, width, (u32int *)window.data);

  /*writes window.data to the double buffer*/
  write_buffer(x, y, width, height, (u32int*)window.data);
  //~ write_buffer(x, y, 200, 200, (u32int*)window.data);

  refreshScreen_VESA();

  //And set window focus
  current_window = window;
  window_list[window.id] = current_window;

  return window;
}

/*creates a standard window with title, border, etc.*/
window_t createWindow(char *windowname, int x, int y, int priority, int width, int height, window_t parent)
{
  static window_t window;
  window.name = windowname;
  window.x = x;
  window.y = y;

  if(parent.id != desktop.id) //if the parent is not the desktop, ie:user wants to create a new object not a sub object
    window.z = parent.z + ((float)priority / MAX_CHILDREN);
  else
    window.z = priority;

  window.width = width;
  window.height = height;
  window.parentid = parent.id;
  window.id = getnewwid();
  window.data = (u32int*)kmalloc((width * height) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0xff, (width * height) * (dVESA / 8));

  ///* Fill */
  plot_BufRect(0, 10, width, height - 10, FALSE, 0, WINDOW_COLOUR_BACKGROUND, width, (u32int *)window.data);


  ///* Topbar */
  plot_BufRect(0, 0, width, 10, FALSE, 0, WINDOW_COLOUR_TOPBAR, width, (u32int*)window.data);

  ///* Border */
  int i;
  for(i = 0; i <= width; i++)
    plot_BufPixel(i, 0, WINDOW_COLOUR_BORDER, width, (u32int*)window.data); //Top

  for(i = 0; i <= width; i++)
    plot_BufPixel(i, height - 1, WINDOW_COLOUR_BORDER, width, (u32int*)window.data); //Bottom

  for(i = 0; i <= height; i++)
  {
    plot_BufPixel(0, i, WINDOW_COLOUR_BORDER, width, (u32int*)window.data); //Left
    plot_BufPixel(width - 1, i, WINDOW_COLOUR_BORDER, width, (u32int*)window.data); //And right...
  }

  ///* Title */
  int windowNameLength = 8 * k_strlen(windowname); //8 * len since letters are 8 pixels wide
  int titlePos = ((window.width) / 2) - (windowNameLength / 2);

  plot_BufString(1 + titlePos, 2, windowname, WINDOW_COLOUR_TOPBAR_TEXT, width, (u32int*)window.data);

  ///* Top Buttons*/
  window.buttons[0] = createButton(1, 1, 9, 9, closeButton, &destroyWindow, window); //X close button
  window.buttons[1] = createButton(10, 1, 9, 9, maximizeButton, &maximizeWindow, window); //maximize button
  window.buttons[2] = createButton(19, 1, 9, 9, minimizeButton, &minimizeWindow, window); //minimize button
  window.buttons[3] = createInvisibleButton(28, 1, width - 28, 9, &topBarMoveWindow); //topbar for moving the window

  /*writes window.data to the double buffer*/
  write_buffer(x, y, width, height, (u32int*)window.data);

  refreshScreen_VESA();

  //And set window focus
  current_window = window;
  window_list[window.id] = current_window;

  return window;
}

window_t createPixbufObject(char *windowname, unsigned long *pixbuf, int x, int y, int priority, int width, int height, window_t parent)
{
  static window_t window;
  window.name = windowname;
  window.x = x;
  window.y = y;

  if(parent.id != desktop.id) //if the parent is not the desktop, ie:user wants to create a new object not a sub object
    window.z = parent.z + ((float)priority / MAX_CHILDREN);
  else
    window.z = priority;

  window.width = width;
  window.height = height;
  window.parentid = parent.id;
  window.id = getnewwid();
  window.data = (u32int*)kmalloc((width * height) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0x0, (width * height) * (dVESA / 8));

  ///* Fill */
  plot_BufRect(0, 0, width, height, TRUE, pixbuf, WINDOW_COLOUR_BACKGROUND, width, (u32int *)window.data);

  ///*writes window.data to the double buffer*/
  write_buffer(x, y, width, height, (u32int*)window.data);

  refreshScreen_VESA();

  //And set window focus
  current_window = window;
  window_list[window.id] = current_window;

  return window;
}

void destroyWindow(window_t windowToDestroy)
{
  int x, y, w, h;

  x = window_list[windowToDestroy.id].x;
  y = window_list[windowToDestroy.id].y;
  w = window_list[windowToDestroy.id].width;
  h = window_list[windowToDestroy.id].height;

  window_list[windowToDestroy.id].data = 0;
  window_list[windowToDestroy.id].name = 0;
  window_list[windowToDestroy.id].id = -1;
  window_list[windowToDestroy.id].x = 0;
  window_list[windowToDestroy.id].y = 0;
  window_list[windowToDestroy.id].z = 0;
  window_list[windowToDestroy.id].width = 0;
  window_list[windowToDestroy.id].height = 0;
  window_list[windowToDestroy.id].parentid = 0;

  refresh_BufArea(x - 3, y - 3, w + 3, h + 3, TRUE);

}

//TODO make topbar movement work
void topBarMoveWindow(window_t windowToMove)
{
  asm volatile("sti");
  init_timer(globalFreq); //Initialise timer to globalFreq-Hz
  
  while(leftButtonPressed == TRUE)
  {
    //~ translateObject(windowToMove.id, 1, 1);
    mSleep(10);
    playNote("C5", 10);
  }
}

void maximizeWindow(window_t windowToMaximize)
{
  window_list[windowToMaximize.id].x = 0;
  window_list[windowToMaximize.id].y = 0;
  window_list[windowToMaximize.id].width = wVESA;
  window_list[windowToMaximize.id].height = hVESA;

  refresh_BufArea(0, 0, wVESA, hVESA, TRUE);

}

void minimizeWindow(window_t windowToMinimize)
{
  window_list[windowToMinimize.id].z = -1;

  refresh_BufObject(windowToMinimize, 3, 3);

}

int isWindowAbovePoint(int x, int y, int index)
{
  if(window_list[index].x > x || window_list[index].y > y)
    return FALSE; //window_list is not over point (x,y)
  else if(window_list[index].x <= x && window_list[index].y <= y &&
          (window_list[index].x + window_list[index].width) > x &&
          (window_list[index].y + window_list[index].height) > y)
    return TRUE; //window_list is over point (x,y)
  else
    return FALSE; //window_list is not over point (x,y)

}


unsigned long int getPixelColorOfObject(int xFromTopLeft, int yFromTopLeft, int index)
{

  int offset = xFromTopLeft + (yFromTopLeft) * window_list[index].width;

  return window_list[index].data[offset];
}


int buttonOnWindow(int x, int y, int indexOfWindow)
{
  int rep;

  for(rep = 0; rep < MAX_BUTTONS; rep++)
  {
    //checks to see if the coordinate (x,y) is ontop of a window's button
    if(window_list[indexOfWindow].x + window_list[indexOfWindow].buttons[rep].x <= x &&
       window_list[indexOfWindow].y + window_list[indexOfWindow].buttons[rep].y <= y &&
       (window_list[indexOfWindow].x + window_list[indexOfWindow].buttons[rep].x +
        window_list[indexOfWindow].buttons[rep].width) > x &&
       (window_list[indexOfWindow].y + window_list[indexOfWindow].buttons[rep].y + 
        window_list[indexOfWindow].buttons[rep].height) > y)
    {
      //return the index of the button
      return rep;

    }

  }

  //if we have not exited yet, return an error
  return -1;

}

int highestWindowAbovePoint(int x, int y, int ignoreObject)
{
  int rep, returnIndex = 0;
  float highestPriority = 0.0;

  for(rep = 0; rep < wid + 1; rep++)
    if(rep != ignoreObject) //ignores the mouse as we want windows not the index of the mouse to always come up
      if(isWindowAbovePoint(x, y, rep) == TRUE && highestPriority <= window_list[rep].z)
      {
        returnIndex = rep;
        highestPriority = window_list[rep].z;
      }

  return returnIndex;

}

int pixelsNextObjectDown(int x, int y, int index)
{
  int objectPriority = window_list[index].z, steps = window_list[index].height;

  int a;
  for(a = 0; a < wid + 1; a++)
  {
    if(a != index)
    {
      if(window_list[a].z >= objectPriority && window_list[a].x <= x && window_list[a].y >= y && //if object has higher priority, x-coord is less than given x, and y-coord is less than given y, respectivly
        window_list[a].x + window_list[a].width > x) //and if the top rightmost x-coord is greater than the given x
      {
        if(steps > window_list[a].y - y) //if the steps is smaller, then return a smaller step
          steps = window_list[a].y - y;
      }
    }
  }

  //if the steps + starting y is greater than the height of the window
  if((y - window_list[index].y) + steps > window_list[index].height) 
    //cap that to the height of the window - original y (amount of pixels untill bottom of window)
    steps = window_list[index].height - (y - window_list[index].y); 

  return steps;

}

void refresh_BufArea(int xLoc, int yLoc, int width, int height, int updateScreen)
{

  int a, x, y;
  float highestPriority = 0.0, previousPriority = 0.0;

  unsigned long int color;

  int lowX = xLoc;
  if(lowX < 0)
    lowX = 0;
  else if(lowX > wVESA)
    lowX = wVESA;

  int highX = xLoc + width;
  if(highX < 0)
    highX = 0;
  else if(highX > wVESA)
    highX = wVESA;

  int lowY = yLoc;
  if(lowY < 0)
    lowY = 0;
  else if(lowY > hVESA)
    lowY = hVESA;

  int highY = yLoc + height;
  if(highY < 0)
    highY = 0;
  else if(highY > hVESA)
    highY = hVESA;

  int index, oldIndex, pixelsDown = 0; // = highestWindowAbovePoint(lowX, lowY);
    for(x = lowX; x < highX; x++)
    {

      for(y = lowY; y < highY; y++)
      {

        if(!pixelsDown)
        {
          index = highestWindowAbovePoint(x, y, -1);
          
          pixelsDown = pixelsNextObjectDown(x, y, index);
        }

        color = getPixelColorOfObject(x - window_list[index].x, y - window_list[index].y, index);

        //TODO this set up for updating area only works with one alpha layer, if there are two alpha layers above each other, this will not work
        while(color == ALPHA)
        {
          oldIndex = index;
          index = highestWindowAbovePoint(x, y, index);
          color = getPixelColorOfObject(x - window_list[index].x, y - window_list[index].y, index);
          index = oldIndex;
        }
        writePixelToDoubleBuffer(x, y, color);

        pixelsDown--; //subtracts one for every y pixel increased as the number of pixels to the next object gets smaller

      }
      pixelsDown = 0; //reset pixelsDown

    }

  if(updateScreen == TRUE)
    refreshArea_VESA(lowX, lowY, highX - lowX, highY - lowY);
}

void refresh_BufObject(window_t object, int xBuffer, int yBuffer)
{
  int a, x, y, indexWithHighestPriority = 0;
  float highestPriority = 0.0;
  unsigned long int color;

  int lowX = object.x - xBuffer;
  if(lowX < 0)
    lowX = 0;
  else if(lowX > wVESA)
    lowX = wVESA;

  int highX = object.width + 2 * xBuffer;
  if(highX < 0)
    highX = 0;
  else if(highX > wVESA)
    highX = wVESA;

  int lowY = object.y - yBuffer;
  if(lowY < 0)
    lowY = 0;
  else if(lowY > hVESA)
    lowY = hVESA;

  int highY = object.height + 2 * yBuffer;
  if(highY < 0)
    highY = 0;
  else if(highY > hVESA)
    highY = hVESA;

  refresh_BufArea(lowX, lowY, highX, highY, TRUE);
}

void translateObject(int index, int xMovement, int yMovement)
{

  if(window_list[index].id != -1)
  {

    int lowX = window_list[index].x + xMovement;
    if(lowX < 0)
      lowX = 0;
    else if(lowX > wVESA)
      lowX = wVESA;

    int lowY = window_list[index].y - yMovement;
    if(lowY < 0)
      lowY = 0;
    else if(lowY > hVESA)
      lowY = hVESA;

    int oldX = window_list[index].x, oldY = window_list[index].y;

    window_list[index].x = lowX;
    window_list[index].y = lowY;

    refresh_BufArea(oldX - 5, oldY - 5, window_list[index].width + 10, window_list[index].height + 10, TRUE);
    refresh_BufArea(lowX - 5, lowY - 5, window_list[index].width + 10, window_list[index].height + 10, TRUE);

  }

}

void refresh_BufScreen()
{

  refresh_BufArea(0, 0, wVESA, hVESA, TRUE);

}

component_t createButton(int x, int y, int width, int height, long int *buttonPixBuf, void (*onLeftClick)(window_t widget), window_t window)
{
  component_t button;

  plot_BufRect(x, y, width, height, TRUE, buttonPixBuf, 0x000000, window.width, (u32int*)window.data);

  button.x = x;
  button.y = y;
  button.width = width;
  button.height = height;
  button.onMouseLeftClick = onLeftClick;

  return button;

}

component_t createInvisibleButton(int x, int y, int width, int height, void (*onLeftClick)(window_t widget))
{
  component_t button;

  button.x = x;
  button.y = y;
  button.width = width;
  button.height = height;
  button.onMouseLeftClick = onLeftClick;

  return button;

}
