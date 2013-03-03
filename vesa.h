#ifndef V86
#define V86

#include "multiboot.h"

#define MAX_BUTTONS   10

//~ typedef int (*callback_type_t)(int x, int y);
typedef void (*callback_type_t)(int);

typedef struct component
{
  int x;
  int y;
  int width;
  int height;
  callback_type_t onMouseLeftClick;
} component_t;

typedef struct window
{
  char *name;
  int id;
  int parentid;
  int x;
  int y;
  int width;
  int height;
  unsigned long *data;
  int z;

  char *flag;
  component_t buttons[MAX_BUTTONS];
} window_t;

//Colors
#define WINDOW_COLOUR_BORDER 0x397D02

#define WINDOW_COLOUR_TOPBAR 0x83F52C
#define WINDOW_COLOUR_FOCUS_TOPBAR 0x7FFF00
#define WINDOW_COLOUR_TOPBAR_TEXT 0x397D02

#define WINDOW_COLOUR_BACKGROUND 0xC5E3BF

/*sets the vesa graphics*/
void setVesa(int mode);

#define 	VBE_DISPI_INDEX_ID   0x0
#define 	VBE_DISPI_INDEX_XRES   0x1
#define 	VBE_DISPI_INDEX_YRES   0x2
#define 	VBE_DISPI_INDEX_BPP   0x3
#define 	VBE_DISPI_INDEX_ENABLE   0x4
#define 	VBE_DISPI_INDEX_BANK   0x5
#define 	VBE_DISPI_INDEX_VIRT_WIDTH   0x6
#define 	VBE_DISPI_INDEX_VIRT_HEIGHT   0x7
#define 	VBE_DISPI_INDEX_X_OFFSET   0x8
#define 	VBE_DISPI_INDEX_Y_OFFSET   0x9
#define 	VBE_DISPI_INDEX_NB   0xa
#define 	VBE_DISPI_ID0   0xB0C0
#define 	VBE_DISPI_ID1   0xB0C1
#define 	VBE_DISPI_ID2   0xB0C2
#define 	VBE_DISPI_DISABLED   0x00
#define 	VBE_DISPI_ENABLED   0x01
#define 	VBE_DISPI_LFB_ENABLED   0x40
#define 	VBE_DISPI_NOCLEARMEM   0x80
#define 	VBE_DISPI_LFB_PHYSICAL_ADDRESS   0xE0000000
#define     VBE_DISPI_IOPORT_INDEX 0x01CE
#define     VBE_DISPI_IOPORT_DATA 0x01CF

/*initializes the Window Manager*/
void init_window_manager();

/*creates an object that is a pixbuf*/
void createPixbufObject(char* windowname, unsigned long *pixbuf, int x, int y, int priority, int width, int height, int parent);

/*destroys a window/object */
void destroyWindow(int id);

/*sees if a window is above a given point*/
int isWindowAbovePoint(int x, int y, int index);

/*gets the color of the pixel (x,y)*/
unsigned long int getPixelColorOfObject(int xFromTopLeft, int yFromTopLeft, int index);

/*refreshes the entire screen*/
void refresh_BufScreen();

/*refresh the an area on the screen*/
void refresh_BufArea(int xLoc, int yLoc, int width, int height, int updateScreen);

/*refreshes the object with a buffer*/
void refresh_BufObject(int object, int xBuffer, int yBuffer);

/*gets the index of the highest window on point (x, y)*/
int highestWindowAbovePoint(int x, int y);

/*Translates an object*/
void translateObject(int index, int xMovement, int yMovement);


/**Window Buttons**/

/*creates a button on a window*/
component_t createButton(int x, int y, int width, int height, long int *buttonPixBuf, void (*onLeftClick)(int), window_t window);
//~ void createButton(int x, int y, int w, int h, char text, window_t window);

/*gets the index of the button on window index under point (x,y)*/
int buttonOnWindow(int x, int y, int indexOfWindow);

/*maximizes the window*/
void maximizeWindow(int id);

/*minimizes the window*/
void minimizeWindow(int id);

/*creates and object with a constant fill*/
void createObject(char *windowname, int x, int y, int priority, int width, int height, int fill, int parent);

/*adds a button to an object*/
void addButtonToObject(int x, int y, int width, int height, long int *pixBuf, void(*call_back)(int), int windowID);

#endif
