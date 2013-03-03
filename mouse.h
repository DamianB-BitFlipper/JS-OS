#ifndef MOUSE
#define MOUSE

/*Handles the mouse interrupt */
void mouseInput_handler();

/*Installs the mouse handler into IRQ12 */
void init_mouse();
//~ void mouse_install();

/*sets/calls functions for left/middle/right click*/
void mouseClickFunctions(char *callOrWrite, char *buttonClick, void (*func)() );

#endif
