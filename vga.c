/* CakeOS */
/* Functions for VGA modes */
//~ #include <system.h>
#include "common.h"
#include "kheap.h"

#include "vga_regs.h"
#include "vga_modes.h"
//~ #include <vga_regs.h>

//~ extern int graphical_mode;
int graphical_mode;

unsigned long int g_wd, g_ht, g_bpp;
unsigned int g_planes, g_pixels;
unsigned int fb_seg;
unsigned char *vga_mem = (unsigned char*)0xA0000;
//~ unsigned char* vga_mem = (unsigned char*)0xA0000;

void (*g_put_pixel)(unsigned, unsigned, unsigned);
void (*g_write_pixel)(unsigned, unsigned, unsigned);
u32int (*g_read_pixel)(int, int);
void (*refresh_screen)(void);

u32int *double_buffer;
u8int *double_buffer_256;

static void setup_vesa_mode(unsigned short width, unsigned short height, unsigned short depth);

static void g_flip(u32int *source, u32int count)
{
    memcpy((u8int*)vga_mem, (u8int*)source, count);
}

/*static void g_partial_flip(u32int *source, u32int x, u32int y, u32int w, u32int h)
{
     for(int i=0; i <= h; i++)
          memcpy((u8int*)vga_mem+(((y*g_wd)+x)+(i*g_wd)), (u8int*)source+(((y*g_wd)+x)+(i*g_wd)), w);
}*/

static unsigned get_fb_seg()
{
        unsigned seg;

        outb(VGA_GC_INDEX, 6);
        seg = inb(VGA_GC_DATA);
        seg >>= 2;
        seg &= 3;
        switch(seg)
        {
        case 0:
        case 1:
                seg = 0xA000;
                break;
        case 2:
                seg = 0xB000;
                //~ vga_mem = (unsigned long*)0xB0000;
                vga_mem = (unsigned char*)0xB0000;
                break;
        case 3:
                seg = 0xB800;
                //~ vga_mem = (unsigned long*)0xB8000;
                vga_mem = (unsigned char*)0xB8000;
                break;
        }
        return seg;
}

static void vsync()
{
      /* wait until any previous retrace has ended */
      do {
      } while (inb(0x3DA) & 8);

      /* wait until a new retrace has just begun */
      do {
      } while (!(inb(0x3DA) & 8));
}

void set_color(int color, int red, int green, int blue)
{
      vsync();
      outb(0x3C8, color);
      outb(0x3C9, red);
      outb(0x3C9, green);
      outb(0x3C9, blue);
}

void refresh_screen_32bpp()
{
     vsync();
     g_flip(double_buffer,(g_wd*g_ht*4));
}

/*void refresh_32bpp(u32int x, u32int y, u32int w, u32int h)
{
     vsync();
     g_partial_flip(double_buffer, x, y, w, h);
}*/

void refresh_screen_256()
{
     vsync();
     g_flip((u32int*)double_buffer_256,(g_wd*g_ht*(g_bpp/8)));
}

/*void refresh_256(u32int x, u32int y, u32int w, u32int h)
{
     vsync();
     g_partial_flip((u32int)double_buffer_256, x, y, w, h);
}*/

//~ static void vpokeb(unsigned off, unsigned val)
//~ {
        //~ pokeb(fb_seg, off, val);
//~ }
//~
//~ static unsigned vpeekb(unsigned off)
//~ {
        //~ return peekl(fb_seg, off);
//~ }
/**Credits to http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
 * This is not mine, but it works. :)*/
#define peekb(S,O)              *(unsigned char *)(16uL * (S) + (O))
#define pokeb(S,O,V)            *(unsigned char *)(16uL * (S) + (O)) = (V)
#define pokew(S,O,V)            *(unsigned short *)(16uL * (S) + (O)) = (V)
#define _vmemwr(DS,DO,S,N)      memcpy((char *)((DS) * 16 + (DO)), S, N)

static void vpokeb(unsigned int off, unsigned int val)
{
  pokeb(get_fb_seg(), off, val);
}

static unsigned vpeekb(unsigned off)
{
  return peekb(get_fb_seg(), off);
}

void plot_pixel(unsigned x,unsigned y, unsigned c, unsigned width, u32int* buffer)
{
     buffer[(width * y + x)] = c;
}

void write_buffer(unsigned x, unsigned y, unsigned width, unsigned height, u32int* buffer)
{
  int j, h;
     for(j = y; j<y + height; j++)
          for(h = x; h<x + width; h++)
               g_write_pixel(h,j,buffer[((j-y)*width)+(h-x)]);
}

void put_buffer(unsigned x, unsigned y, unsigned width, unsigned height, u32int* buffer)
{
  int j, h;

     for(j = y; j<y + height; j++)
          for(h = x; h<x + width; h++){
               g_put_pixel(h,j,buffer[((j-y)*width)+(h-x)]);
          }
}

void put_pixel_32bpp(unsigned x,unsigned y, unsigned c)
{
    vga_mem[(g_wd * y + x)] = c;
}

void write_pixel_32bpp(unsigned x,unsigned y, unsigned c)
{
    double_buffer[g_wd * y + x] = c;
    //~ vga_mem[g_wd * y + x] = c;
}

u32int read_pixel_32bpp(int x,int y)
{
    int offset = g_wd * y + x;
    return vga_mem[offset];
}

void put_pixel_256(unsigned x, unsigned y, unsigned c)
{
    if(x < 0 || x > (int)g_wd) return;
        if(y < 0 || y > (int)g_ht) return;

        vpokeb((g_wd * y + x), c);
}

void write_pixel_256(unsigned x, unsigned y, unsigned c)
{
        if(x < 0 || x > (int)g_wd) return;
        if(y < 0 || y > (int)g_ht) return;

    double_buffer_256[g_wd*y+x] = c;
}


u32int read_pixel_256(int x, int y)
{
        int offset = y * g_wd + x;
        return vpeekb(offset);
}

static void write_regs(unsigned char *regs)
{
        unsigned i;

/* write MISCELLANEOUS reg */
        outb(VGA_MISC_WRITE, *regs);
        regs++;
/* write SEQUENCER regs */
        for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
        {
                outb(VGA_SEQ_INDEX, i);
                outb(VGA_SEQ_DATA, *regs);
                regs++;
        }
/* unlock CRTC registers */
        outb(VGA_CRTC_INDEX, 0x03);
        outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
        outb(VGA_CRTC_INDEX, 0x11);
        outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
        regs[0x03] |= 0x80;
        regs[0x11] &= ~0x80;
/* write CRTC regs */
        for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
        {
                outb(VGA_CRTC_INDEX, i);
                outb(VGA_CRTC_DATA, *regs);
                regs++;
        }
/* write GRAPHICS CONTROLLER regs */
        for(i = 0; i < VGA_NUM_GC_REGS; i++)
        {
                outb(VGA_GC_INDEX, i);
                outb(VGA_GC_DATA, *regs);
                regs++;
        }
/* write ATTRIBUTE CONTROLLER regs */
        for(i = 0; i < VGA_NUM_AC_REGS; i++)
        {
                (void)inb(VGA_INSTAT_READ);
                outb(VGA_AC_INDEX, i);
                outb(VGA_AC_WRITE, *regs);
                regs++;
        }
/* lock 16-color palette and unblank display */
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, 0x20);
}

void draw_rect(int x, int y, int w, int h, int c)
{
  int j, i;
     for(j = y; j < (y+h); j++)
          for(i = x; i < (x+w); i++)
               g_write_pixel(i, j, c);

     refresh_screen();
}

void set_vga_mode(int mode)
{
    switch(mode)
    {
    case VGA_MODE_13h:
         graphical_mode = 1;
             write_regs(VGA_320_200_256);
             g_wd = 320;
             g_ht = 200;
             g_bpp = 16;
             g_planes = 1;
             g_pixels = 8;
             g_put_pixel = put_pixel_256;
             g_write_pixel = write_pixel_256;
             g_read_pixel = read_pixel_256;
             refresh_screen = refresh_screen_256;
             //refresh = refresh_256;
             fb_seg = get_fb_seg();
    break;
    case VESA_MODE_809:
         graphical_mode = 2;
             g_wd = 600;
             g_ht = 480;
             g_bpp = 32;
             setup_vesa_mode(600, 480, 32);
             g_planes = 1;
             g_pixels = 8;
             g_put_pixel = put_pixel_32bpp;
             g_write_pixel = write_pixel_32bpp;
             g_read_pixel = read_pixel_32bpp;
             refresh_screen = refresh_screen_32bpp;
             //refresh = refresh_32bpp;
    break;
    case VESA_MODE_824:
         graphical_mode = 2;
             g_wd = 1024;
             g_ht = 768;
             g_bpp = 32;
             setup_vesa_mode(1024, 768, 32);
             g_planes = 1;
             g_pixels = 8;
             g_put_pixel = put_pixel_32bpp;
             g_write_pixel = write_pixel_32bpp;
             g_read_pixel = read_pixel_32bpp;
             refresh_screen = refresh_screen_32bpp;

             //refresh = refresh_32bpp;
             //g_displaybmp = displaybmp_256;
    break;
        default:
           //Unspecified mode supplied
    break;
    }

    if(graphical_mode == 2)
    {
	//Create the double buffer

        //~ int address;

        k_printf("\n\n%d\t%d\n\n", g_wd, g_ht);


        //~ double_buffer = (u32int*)kmalloc((g_wd * g_ht) * 4);
        //~ address = kmalloc((g_wd * g_ht) * 4);
        //~ address = kmalloc(786432);

        k_printf("\n\n%h\n\n", double_buffer);

	//~ while(1);

        memset((u8int*)double_buffer, 0, g_wd * g_ht * (g_bpp / 8));
    }else{
        double_buffer_256 = (u8int*)kmalloc(g_wd * g_ht * (g_bpp / 8));
        memset(double_buffer_256, 0, g_wd * g_ht * (g_bpp / 8));
    }

    /* Clear to black */
    draw_rect(0,0,g_wd,g_ht,0xffffff00);

    return;
}

static void setup_vesa_mode(unsigned short width, unsigned short height, unsigned short depth)
{
  //soon to be some fancy hardwear detection and
  //therefore driver loading.
  //for now we will assume bochs/qemu
  //~ vga_mem = (unsigned long int*)0;

  /**UNCOMMENT**/
  vga_mem = (unsigned char*)0;

  bochs_vbe_set_mode(width,height,depth);

  while(vga_mem == 0)
  {
    __asm__ __volatile__ ("nop");
  }

  //~ memset((u8int*)vga_mem , 0, g_wd * g_ht * (g_bpp/8));
  memset(vga_mem , 0, g_wd * g_ht * (g_bpp/8));
  /**UNCOMMENT**/

  //~ memset((u8int*)vga_mem , 0, 1024 * 768 * 4);
  //~ memset(vga_mem , 0, g_wd * g_ht * (g_bpp/8));
}

/////////////////////////////////////////////////////
void setcolour(int x, int y, int r, int g, int b);

void setcolour(int x, int y, int r, int g, int b)
{
   u32int colour1, colour2, colour3=0;

   if (r > 255) r = 255; if (r < 0) r = 0;
   if (g > 255) g = 255; if (g < 0) g = 0;
   if (b > 255) b = 255; if (b < 0) b = 0;

   //Shift the RGB bits and assign them to
   //a 32 bit word.
   colour1 = r << 16;
   colour2 = g << 8;
   colour3 = b;
   //Now they're in the correct position, combine
   //them using OR's
   colour1 |= colour2;
   colour1 |= colour3;
   k_printf(" 0x%d (%d %d %d) ",colour1,r,g,b);
   return;
}

