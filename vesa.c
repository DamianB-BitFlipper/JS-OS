
#include "common.h"
#include "kheap.h"
#include "k_stdio.h"
#include "graphics.h"
#include "x_server.h"
#include "timer.h"
#include "vesa.h"

extern int globalFreq;

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


// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);

/*Sets the bank if the Linear Frame Buffer is not supported/enabled*/
void setBank(int bankNo)
{
	regs16_t regs;

  regs.ax = 0x4f05;
  regs.bx = 0x0;
  regs.dx = bankNo;

  int32(0x10, &regs);

}

//sets up VESA for mode
void setVesa(int mode)
{

  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz


  VESA_INFO info; //VESA information
  MODE_INFO vbeModeInfo; //VESA mode information

  regs16_t regs;

  /**Gets VESA information**/

  int buffer = kmalloc(sizeof(VESA_INFO)) & 0xFFFFF; //sets the address for the buffer

  memcpy(buffer, "VBE2", 4);
  memset(&regs,0,sizeof(regs)); //clears the registers typedef struct

  regs.ax = 0x4f00; //mode that gets VESA information
  regs.di = buffer & 0xF;
  regs.es = (buffer>>4) & 0xFFFF;
  int32(0x10, &regs); //calls v86 interupt
  memcpy(&info, buffer, sizeof(VESA_INFO)); //copies info from the buffer to the info typedef struct

  k_printf("\n\nVesa Signature: %s\n", info.VESASignature);
  k_printf("\n\nVesa Version: %h\n", info.VESAVersion);
  k_printf("\n\nVesa Video Modes: %h\n", info.VideoModePtr);

  //~ while(1);

  /**Gests VESA mode information**/

  int modeBuffer = kmalloc(sizeof(MODE_INFO)) & 0xFFFFF; //sets the address for the buffer

  memset(&regs,0,sizeof(regs)); //clears the registers typedef struct

	regs.ax = 0x4f01; //mode the gets the VESA mode information
	regs.di = modeBuffer & 0xF;
	regs.es = (modeBuffer>>4) & 0xFFFF;
	regs.cx = mode; //mode to get the information for
	int32(0x10, &regs);
  memcpy(&vbeModeInfo, modeBuffer, sizeof(MODE_INFO));

  widthVESA = vbeModeInfo.XResolution;
  heightVESA = vbeModeInfo.YResolution;
  depthVESA = vbeModeInfo.BitsPerPixel;

  k_printf("\nBase Pointer: %h\n", (int)vbeModeInfo.PhysBasePtr);
  k_printf("\nXRes: %d\n", (int)vbeModeInfo.XResolution);
  k_printf("\nYRes: %d\n", (int)vbeModeInfo.YResolution);
  k_printf("\nBits per pixel: %d\n", (int)vbeModeInfo.BitsPerPixel);
  k_printf("\nExits status: %h\n", (int)regs.ax);

  /*Sets the Linear Frame Buffer address tp vga_mem and lfb variables*/
  vga_mem = (unsigned char*)vbeModeInfo.PhysBasePtr;
  int lfb = (int)vbeModeInfo.PhysBasePtr;

  //~ while(1);

  /**Sets up the VESA mode**/
	regs.ax = 0x4f02; //mode the sets up VESA graphics
	regs.bx = (mode | 0x4000); //sets up mode with a linear frame buffer
	int32(0x10, &regs);

  //int x = 0;
	////full screen with blue color (1)
  //while(1)
  //{
    //vga_mem[x] = 0xbe;
    //vga_mem[x + 1] = 0xfd;
    //vga_mem[x + 2] = 0x98;
    ////~ mSleep(1);
    //x = x + (depthVESA / 8);
  //}


	//// wait for key
	//regs.ax = 0x0000;
	//int32(0x16, &regs);

	//// switch to 80x25x16 text mode
	//regs.ax = 0x0003;
	//int32(0x10, &regs);
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
  //~ window.data = (unsigned long*)double_buffer;
  window.data = (unsigned long*)kmalloc((wVESA * hVESA) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0xff, (wVESA * hVESA) * (dVESA / 8));

  //~ unsigned int i;
  //~ for(i = 0; i < (wVESA * hVESA) * (dVESA / 8); i++)
  //~ {
    //~ *(window.data + i) = 0xff;
  //~ }

  k_printf("\n background data address: %h", window.data);

  //~ int a;
  //~ for(a = 0; a < (wVESA * hVESA) * (dVESA / 8); a++)
  //~ {
    //~ *(window.data + a) = 0xff;
  //~ }

  //~ while(1);


  //~ plot_BufRect(0, 0, 1024, 768, FALSE, 0, 0xffffff, 1024, (u32int*)window.data);
//~
  write_buffer(0, 0, 1024, 768, (u32int*)window.data);
  refreshScreen_VESA();
  //~ while(1);

  current_window = window;

  window_list = (window_t*)kmalloc(sizeof(window_t) * MAX_WINDOWS);
  window_list[window.id] = current_window;
}

/*gets the new id for a window*/
static int getnewwid()
{
  wid++;
  return wid;
}

void addButtonToObject(int x, int y, int width, int height, long int *pixBuf, void(*call_back)(int), int windowID)
{
  int buttonNumber, tmp, assigned = FALSE;

  for(tmp = 0; tmp < MAX_BUTTONS; tmp++)
  {
    if(window_list[windowID].buttons[tmp].onMouseLeftClick == 0)
    {
      buttonNumber = tmp;
      assigned = TRUE;
      break;
    }
  }

  if(assigned == TRUE)
  {
    //~ window_list[windowID].buttons[buttonNumber].onMouseLeftClick = call_back;
    window_list[windowID].buttons[buttonNumber] = createButton(x, y, width, height, pixBuf, call_back, window_list[windowID]);
    refresh_BufArea(window_list[windowID].x + x, window_list[windowID].y + y, width, height, TRUE);
  }
}

/*creates a object with a constant fill*/
void createObject(char *windowname, int x, int y, int priority, int width, int height, int fill, int parent)
{
  static window_t window;
  window.name = windowname;
  window.x = x;
  window.y = y;
  window.z = priority;
  window.width = width;
  window.height = height;
  window.parentid = parent;
  window.id = getnewwid();
  window.data = (unsigned long*)kmalloc((width * height) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0xff, (width * height) * (dVESA / 8));

  ///* Fill */
  plot_BufRect(0, 0, width, height, FALSE, 0, fill, width, (u32int *)window.data);

  //window.buttons[0] = *createButton(1, 1, 9, 9, closeButton, &destroyWindow, window);
  //window.buttons[1] = *createButton(10, 1, 9, 9, maximizeButton, &maximizeWindow, window);
  //window.buttons[2] = *createButton(19, 1, 9, 9, minimizeButton, &minimizeWindow, window);
  

  /*writes window.data to the double buffer*/
  write_buffer(x, y, width, height, (u32int*)window.data);
  //~ write_buffer(x, y, 200, 200, (u32int*)window.data);

  refreshScreen_VESA();

  //And set window focus
  current_window = window;
  window_list[window.id] = current_window;
}

/*creates a standard window with title, border, etc.*/
void createWindow(char *windowname, int x, int y, int priority, int width, int height, int parent)
{
  static window_t window;
  window.name = windowname;
  window.x = x;
  window.y = y;
  window.z = priority;
  window.width = width;
  window.height = height;
  window.parentid = parent;
  window.id = getnewwid();
  window.data = (unsigned long*)kmalloc((width * height) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0xff, (width * height) * (dVESA / 8));

  ///* Fill */
  plot_BufRect(0, 10, width, height - 10, FALSE, 0, WINDOW_COLOUR_BACKGROUND, width, (u32int *)window.data);


  ///* Topbar */
  plot_BufRect(0, 0, width, 10, FALSE, 0, WINDOW_COLOUR_TOPBAR, width, (u32int*)window.data);

  ///* Border */
  int i;
  for(i = 0; i <= width; i++)
  {
    plot_BufPixel(i, 0, WINDOW_COLOUR_BORDER, width, (u32int*)window.data); //Top
  }

  for(i = 0; i <= width; i++)
  {
    plot_BufPixel(i, height - 1, WINDOW_COLOUR_BORDER, width, (u32int*)window.data); //Bottom
  }

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
  window.buttons[0] = createButton(1, 1, 9, 9, closeButton, &destroyWindow, window);
  window.buttons[1] = createButton(10, 1, 9, 9, maximizeButton, &maximizeWindow, window);
  window.buttons[2] = createButton(19, 1, 9, 9, minimizeButton, &minimizeWindow, window);
  //~ createButton(10, 10, 100, 100, 'X', window);
  //~ plot_BufRect(10, 10, 100, 100, FALSE, 0, 0x000000, window.width, (u32int*)window.data);
  

  /*writes window.data to the double buffer*/
  write_buffer(x, y, width, height, (u32int*)window.data);
  //~ write_buffer(x, y, 200, 200, (u32int*)window.data);

  refreshScreen_VESA();

  //And set window focus
  current_window = window;
  window_list[window.id] = current_window;
}

void createPixbufObject(char *windowname, unsigned long *pixbuf, int x, int y, int priority, int width, int height, int parent)
{
  static window_t window;
  window.name = windowname;
  window.x = x;
  window.y = y;
  window.z = priority;
  window.width = width;
  window.height = height;
  window.parentid = parent;
  window.id = getnewwid();
  window.data = (unsigned long*)kmalloc((width * height) * (dVESA / 8)); //Creates buffer for window

  memset(window.data, 0x0, (width * height) * (dVESA / 8));

  ///* Fill */
  plot_BufRect(0, 0, width, height, TRUE, pixbuf, WINDOW_COLOUR_BACKGROUND, width, (u32int *)window.data);

  ///*writes window.data to the double buffer*/
  write_buffer(x, y, width, height, (u32int*)window.data);

  refreshScreen_VESA();

  //And set window focus
  current_window = window;
  window_list[window.id] = current_window;
}

void destroyWindow(int id)
{
  //~ kfree(window_list[id].data);

  int x, y, w, h;

  x = window_list[id].x;
  y = window_list[id].y;
  w = window_list[id].width;
  h = window_list[id].height;

  window_list[id].data = 0;
  window_list[id].name = 0;
  window_list[id].id = -1;
  window_list[id].x = 0;
  window_list[id].y = 0;
  window_list[id].z = 0;
  window_list[id].width = 0;
  window_list[id].height = 0;
  window_list[id].parentid = 0;

  refresh_BufArea(x - 3, y - 3, w + 3, h + 3, TRUE);

  //~ write_buffer(window_list[0].x, window_list[0].y, window_list[0].width, window_list[0].height, (u32int*)window_list[0].data);    
  //~ refreshArea_VESA(x, y, w, h);

  
}

void maximizeWindow(int id)
{
  window_list[id].x = 0;
  window_list[id].y = 0;
  window_list[id].width = wVESA;
  window_list[id].height = hVESA;

  refresh_BufArea(0, 0, wVESA, hVESA, TRUE);
  
}

void minimizeWindow(int id)
{
  window_list[id].z = -1;

  refresh_BufObject(id, 3, 3);
  
}

int isWindowAbovePoint(int x, int y, int index)
{
  if(window_list[index].x > x || window_list[index].y > y)
  {
    return FALSE; //window_list is not over point (x,y)
  //~ }else if(window_list[index].x <= x && window_list[index].y <= y && (window_list[index].x + window_list[index].width) >= x && (window_list[index].y + window_list[index].height) >= y)
  }else if(window_list[index].x <= x && window_list[index].y <= y && (window_list[index].x + window_list[index].width) > x && (window_list[index].y + window_list[index].height) > y)
  {
    return TRUE; //window_list is over point (x,y)
  }else{
    return FALSE; //window_list is not over point (x,y)
  }

}


unsigned long int getPixelColorOfObject(int xFromTopLeft, int yFromTopLeft, int index)
{

  int offset = xFromTopLeft + (yFromTopLeft) * window_list[index].width;

  return window_list[index].data[offset];
}


int buttonOnWindow(int x, int y, int indexOfWindow)
{
  int rep, ifHasReturned = FALSE;

  for(rep = 0; rep < MAX_BUTTONS; rep++)
  {
    if(window_list[indexOfWindow].x + window_list[indexOfWindow].buttons[rep].x <= x && window_list[indexOfWindow].y + window_list[indexOfWindow].buttons[rep].y <= y &&
    (window_list[indexOfWindow].x + window_list[indexOfWindow].buttons[rep].x + window_list[indexOfWindow].buttons[rep].width) > x &&
    (window_list[indexOfWindow].y + window_list[indexOfWindow].buttons[rep].y + window_list[indexOfWindow].buttons[rep].height) > y)
    {
      ifHasReturned = TRUE;
      return rep; //button in window_list is over point (x,y)

      break;
    }

  }

  if(ifHasReturned == FALSE)
  {
    return -1;
  }
  
}

int highestWindowAbovePoint(int x, int y)
{
  int rep, highestPriority = 0, returnIndex = 0;

  for(rep = 0; rep < wid + 1; rep++)
  {
    if(rep != MOUSE_INDEX) //ignores the mouse as we want windows not the index of the mouse to always come up
    {
      if(isWindowAbovePoint(x, y, rep) == TRUE && highestPriority <= window_list[rep].z)
      {
        returnIndex = rep;
        highestPriority = window_list[rep].z;
      }
    }
  }

  return returnIndex;
  
}


void refresh_BufArea(int xLoc, int yLoc, int width, int height, int updateScreen)
{

  int a, highestPriority = 0, x, y;
  unsigned long int color;

  int lowX = xLoc;
  if(lowX < 0)
  {
    lowX = 0;
  }else if(lowX > wVESA)
  {
    lowX = wVESA;
  }

  int highX = xLoc + width;
  if(highX < 0)
  {
    highX = 0;
  }else if(highX > wVESA)
  {
    highX = wVESA;
  }

  int lowY = yLoc;
  if(lowY < 0)
  {
    lowY = 0;
  }else if(lowY > hVESA)
  {
    lowY = hVESA;
  }

  int highY = yLoc + height;
  if(highY < 0)
  {
    highY = 0;
  }else if(highY > hVESA)
  {
    highY = hVESA;
  }


  for(a = 0; a < wid + 1; a++)
  {
    for(x = lowX; x < highX; x++)
    {

      for(y = lowY; y < highY; y++)
      {
        if(highestPriority <= window_list[a].z)
        {
          if(isWindowAbovePoint(x, y, a) == TRUE)
          {
            color = getPixelColorOfObject(x - window_list[a].x, y - window_list[a].y, a);
            //~ plot_BufPixel(x - window_list[a].x, y - window_list[a].y, color, window_list[a].width, (u32int*)window_list[a].data);
            writePixelToDoubleBuffer(x, y, color);
            highestPriority = window_list[a].z;
          }
        }

      }

    }
  }

  //~ write_buffer(window_list[object].x, window_list[object].y, window_list[object].width, window_list[object].height, (u32int*)window_list[object].data);
  if(updateScreen == TRUE)
  {
    refreshArea_VESA(lowX, lowY, highX - lowX, highY - lowY);
  }
  //~ refreshScreen_VESA();
}

void refresh_BufObject(int object, int xBuffer, int yBuffer)
{
  //~ int array[wid + 1];
//~
  //~ objectsUnderObject(array, object);

  int a, highestPriority = 0, x, y;
  unsigned long int color;

  int lowX = window_list[object].x - xBuffer;
  if(lowX < 0)
  {
    lowX = 0;
  }else if(lowX > wVESA)
  {
    lowX = wVESA;
  }

  int highX = window_list[object].x + window_list[object].width + xBuffer;
  if(highX < 0)
  {
    highX = 0;
  }else if(highX > wVESA)
  {
    highX = wVESA;
  }

  int lowY = window_list[object].y - yBuffer;
  if(lowY < 0)
  {
    lowY = 0;
  }else if(lowY > hVESA)
  {
    lowY = hVESA;
  }

  int highY = window_list[object].y + window_list[object].height + yBuffer;
  if(highY < 0)
  {
    highY = 0;
  }else if(highY > hVESA)
  {
    highY = hVESA;
  }


  for(a = 0; a < wid + 1; a++)
  {
    for(x = lowX; x < highX; x++)
    {

      for(y = lowY; y < highY; y++)
      {
        if(highestPriority <= window_list[a].z)
        {
          if(isWindowAbovePoint(x, y, a) == TRUE)
          {
            color = getPixelColorOfObject(x - window_list[a].x, y - window_list[a].y, a);
            //~ plot_BufPixel(x - window_list[a].x, y - window_list[a].y, color, window_list[a].width, (u32int*)window_list[a].data);
            writePixelToDoubleBuffer(x, y, color);
            highestPriority = window_list[a].z;
          }
        }

        //if(array[a] != -1)
        //{
          ////~ write_buffer(window_list[a].x, window_list[a].y, window_list[a].width, window_list[a].height, (u32int*)window_list[a].data);
          //write_locationBuffer(window_list[a].x, window_list[a].y, window_list[object].x - window_list[a].x, window_list[object].y - window_list[a].y, window_list[a].width, window_list[a].height, (u32int*)window_list[a].data);
        //}
      }

    }
  }

  //~ write_buffer(window_list[object].x, window_list[object].y, window_list[object].width, window_list[object].height, (u32int*)window_list[object].data);

  refreshArea_VESA(lowX, lowY, highX - lowX, highY - lowY);

  //~ refreshScreen_VESA();
}


void translateObject(int index, int xMovement, int yMovement)
{

  if(window_list[index].id != -1)
  {

    int lowX = window_list[index].x + xMovement;
    if(lowX < 0)
    {
      lowX = 0;
    }else if(lowX > wVESA)
    {
      lowX = wVESA;
    }

    int lowY = window_list[index].y - yMovement;
    if(lowY < 0)
    {
      lowY = 0;
    }else if(lowY > hVESA)
    {
      lowY = hVESA;
    }


    //~ window_list[index].x = window_list[index].x + xMovement;
    //~ window_list[index].y = window_list[index].y - yMovement;
    int oldX = window_list[index].x, oldY = window_list[index].y;

    window_list[index].x = lowX;
    window_list[index].y = lowY;

    //~ refresh_BufScreen();
    //~ refresh_BufObject(index, math_abs((int)(xMovement * 1.5)), math_abs((int)(yMovement * 1.5)));
    //~ refresh_BufObject(index, math_abs(xMovement) + 5, math_abs(yMovement) + 5);
    refresh_BufArea(oldX - 5, oldY - 5, window_list[index].width + 5, window_list[index].height + 5, TRUE);
    refresh_BufObject(index, 5, 5);

  }

}

void refresh_BufScreen()
{
  //int id;

  //int priority[wid + 1][2];

  //for(id = 0; id < wid + 1; id++)
  //{

    //priority[id][0] = window_list[id].z;
    //priority[id][1] = id;
    ////~ if(window_list[id].id != -1) //if this object is real and not deleted
    ////~ {
      ////~ write_buffer(window_list[id].x, window_list[id].y, window_list[id].width, window_list[id].height, (u32int*)window_list[id].data);
    ////~ }

  //}

  //int one, two, idONE, idTWO;

  //int a, b;

  //if(wid > 0)
  //{
    //for(b = 0; b < wid; b++)
    //{
      //for(a = 0; a < wid; a++)
      //{
        //one = priority[a][0];
        //two = priority[a + 1][0];

        //if(one > two)
        //{
          //idONE = priority[a][1];
          //idTWO = priority[a + 1][1];

          //priority[a][0] = two;
          //priority[a][1] = idTWO;

          //priority[a + 1][0] = one;
          //priority[a + 1][1] = idONE;
        //}
      //}
    //}

  //}

  //for(id = 0; id < wid + 1; id++)
  ////~ for(id = 0; id < 1; id++)
  //{

    //if(window_list[ priority[id][1] ].id != -1) //if this object is real and not deleted
    //{
      //write_buffer(window_list[ priority[id][1] ].x, window_list[ priority[id][1] ].y, window_list[ priority[id][1] ].width, window_list[ priority[id][1] ].height, (u32int*)window_list[ priority[id][1] ].data);
    //}

  //}

  //refreshScreen_VESA();

  refresh_BufArea(0, 0, wVESA, hVESA, TRUE);

}



component_t createButton(int x, int y, int width, int height, long int *buttonPixBuf, void (*onLeftClick)(int), window_t window)
//~ void createButton(int x, int y, int w, int h, char text, window_t window)
{
  component_t button;
  
  //~ plot_BufRect(x, y, w, h, FALSE, 0, 0x000000, window_list[indexOfWindow].width, (u32int*)window_list[indexOfWindow].data);
  //~ plot_BufRect(x, y, w, h, FALSE, 0, 0x000000, window.width, (u32int*)window.data);
  plot_BufRect(x, y, width, height, TRUE, buttonPixBuf, 0x000000, window.width, (u32int*)window.data);

  button.x = x;
  button.y = y;
  button.width = width;
  button.height = height;
  button.onMouseLeftClick = onLeftClick;

  return button;

}
