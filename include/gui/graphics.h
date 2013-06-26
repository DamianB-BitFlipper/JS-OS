/*
 * graphics.h
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

#ifndef GRAPHICS
#define GRAPHICS

#include <system.h>

typedef struct objects{
  char *name;
  int x;
  int y;
  int width;
  int height;
  int color;
  
  int borderColor; //border
  int borderSize;

  int shouldHide; //if window should hide or not
  
  float priority; //priority
  float belongsToIndex; //belongs to parent with index

//-------Signals-------//
  void (*onLeftClick)();
  void (*onMiddleClick)();
  void (*onRightClick)();
  
}objects;

//typedef struct subObjects{
  ////~ int x;
  ////~ int y;
  ////~ int width;
  ////~ int height;
  ////~ int color;
  ////~ int priority;
////~ 
////~ //-------Signals-------//
  ////~ void (*onLeftClick)();
  ////~ void (*onMiddleClick)();
  ////~ void (*onRightClick)();
  //objects subs[10];
  
//}subObjects;

#define ALPHA   -2
#define PLOT_ALPHA   -3

/*Initialize videomode*/
void VGA_init(int width, int height, int bpp);

/*Initialize textmode*/
void set_text_mode(int hi_res);

/*place a pixel on the screen with location x,y and color*/
void putPixel_simpleStd(int x, int y, int color);

/*place rectangle on screen with top left (x,y) co-ord, width, length, and fill color*/
void putRect(int x, int y, int width, int height, int fill);

/*place rectangle on screen with top left (x,y) co-ord, width, length, and pixbuf as the color*/
void putPixbufRect(int x, int y, int width, int height, int *pixbuf);

/*gets the width of a pixbuf*/
int getPixbufWidth(int *pixbuf);

/*gets the height of a pixbuf*/
int getPixbufHeight(int *pixbuf);

/*place line starting at (x1,y1) to (x2,y2) with color of fill*/
void putLine(int x1, int y1, int x2, int y2, int fill);

/*puts a horizontal or verticle line, "H" or "V" at (x,y) with length*/
void putHVLine(int x, int y, int length, int color, char *HorV);

/*Draws the border of an object*/
void putBorderOnObject(objects *object, int index);

/*Puts a char at (x,y) while in graphical mode*/
void putGraphicChar(char *letter, int x, int y, int color, int fontSize);

/*Puts a string starting at (x, y) while in graphical mode*/
void putGraphicString(char *string, int x, int y, int color, int fontSize);

/*clears the screen, makes it all white*/
void VGA_clear_screen();

/*Refresh an array of objects*/
void refreshObjects(objects *number, int numberOfObjects, int indexToRefresh, int xBuffer, int yBuffer, int *pixbuf);
void refreshScreen_std(objects *number, int numberOfObjects);
void refreshArea(objects *number, int numberOfObjects, int topLeftX, int topLeftY, int width, int height);

/*sees if object is above a point given*/
int isObjectAbovePoint(int x, int y, objects *object, int index);
int isAttributeAbovePoint(int pointX, int pointY, int x, int y, int width, int height);

/*sees if two objects have collided*/
int objectsCollision(objects *object, int firstIndex, int secondIndex, int xBufferCollision, int yBufferCollision);

/*returns the index of the object directly below the index given in*/
int indexOfObjectBelow(objects *number, int numberOfObjects, int indexOfObjectToLookUnder, int x, int y);

/*returns the index of the highest object above a point input*/
int indexOfHighestObjectAbovePoint(int x, int y, objects *object, int numberOfObjects);

/*VGA inits*/
static void set_plane(unsigned p);
static void vpokeb(unsigned int off, unsigned int val);
static unsigned vpeekb(unsigned off);

/*Sync buffer*/
static void vsync();

/*Used to copy double buffer to vga_mem*/
static void g_flip(unsigned char *source, u32int count);

/*VESA*/
void putPixel_VESA(int x, int y, int RGB);

/*Refresh VESA screen*/
void refreshScreen_VESA();
void refreshArea_VESA(int x, int y, int width, int height);

/*Plot buffered Rect and Pixel*/
void plot_BufRect(int x, int y, int width, int height, int isPixbuf, long *pixbuf, int fill, int widthOfWholeWindow, u32int *buffer);
void plot_BufPixel(int x, int y, int RGB, int width, u32int *buffer);

/*writes buffer to the double buffer*/
void writePixelToDoubleBuffer(unsigned x, unsigned y, unsigned RGB);
void write_buffer(unsigned x, unsigned y, unsigned width, unsigned height, u32int *buffer);

/*Writes text to buffer*/
void plot_BufString(int x, int y, char *string, int color, int widthOfWholeWindow, u32int *buffer);


#endif
