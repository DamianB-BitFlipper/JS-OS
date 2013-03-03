/* CakeOS */
/* VGA/VESA mode numbers */

#ifndef VGA_MODES_H
#define VGA_MODES_H

//320x200 256 colour
#define VGA_MODE_13h 0x13

//640x480x32
#define VESA_MODE_809 0x809

//1024x768x32
#define VESA_MODE_824 0x824

void set_vga_mode(int mode);


#endif
