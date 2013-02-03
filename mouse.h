#ifndef MOUSE
#define MOUSE

/* Handles the mouse interrupt */
void mouseInput_handler();

/* Installs the mouse handler into IRQ12 */
void init_mouse();

#endif
