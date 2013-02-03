#ifndef GRAPHICS
#define GRAPHICS

typedef struct{
  int x;
  int y;
  int width;
  int height;
  int color;
  int priority;
  
}objects;

/*Initialize videomode*/
void VGA_init(int width, int height, int bpp);

/*Initialize textmode*/
void set_text_mode(int hi_res);

/*place a pixel on the screen with location x,y and color*/
void putPixel(int x, int y, int color);

/*place rectangle on screen with top left (x,y) co-ord, width, length, and fill color*/
void putRect(int x, int y, int width, int height, int fill);

/*place line starting at (x1,y1) to (x2,y2) with color of fill*/
void putLine(int x1, int y1, int x2, int y2, int fill);

/*clears the screen, makes it all white*/
void VGA_clear_screen();

/*Refresh an array of objects*/
void refreshObjects(objects *number, int numberOfObjects, int indexToRefresh, int xBuffer, int yBuffer);
void refreshScreen(objects *number, int numberOfObjects);

/*sees if object is above a point given*/
int isObjectAbovePoint(int x, int y, objects *object, int index);
int isAttributeAbovePoint(int pointX, int pointY, int x, int y, int width, int height);

/*sees if two objects have collided*/
int objectsCollision(objects *object, int firstIndex, int secondIndex, int xBufferCollision, int yBufferCollision);

#endif
