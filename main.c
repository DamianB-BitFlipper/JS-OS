// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "k_stdio.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "keyboard.h"
#include "mouse.h"

#include "k_shell.h"
#include "graphics.h"

#include "k_programs.h"

struct multiboot *mboot_ptr;

 //Play sound using built in speaker
 static void play_sound(u32int nFrequence) {
 	u32int Div;
 	u8int tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (u8int) (Div) );
 	outb(0x42, (u8int) (Div >> 8));
 
        //And play the sound using the PC speaker
 	tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
      outb(0x61, tmp | 3);
    }
}
 
 //make it shutup
static void nosound() {
 	u8int tmp = (inb(0x61) & 0xFC);
 
 	outb(0x61, tmp);
}
 
 //Make a beep
void beep() {
  play_sound(1000);
 	mSleep(500);
 	nosound();
  //set_PIT_2(old_frequency);
}

int main()
{
  int clockFreq = 1000; //initialized clock to 1000 ticks per second

  
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
  
  // Initialise the screen (by clearing it)
  k_clear();
  // Write out a sample string
  //~ monitor_write("Hello, world----!");
  //~ k_printf("Heeeeello, wor%dld %d ----333", 5000, 360);
  //~ k_printf(

  setScreenYMinMax(1, 25); //reserve 1 row at the top for OS name

  k_setprintf(0, 0, "%Cw%cbk  JS-OS 0.0.1                                                                    %Cbk%cw ");
  
  k_printf("Hello World");
  k_printf("\nWelcome to JS-OS, Kernel booted. Running OS.\tAnd thus i poop\n");

  asm volatile("int $0x3");
  asm volatile("int $0x4");

  asm volatile("sti");
  init_timer(clockFreq); // Initialise timer to clockFreq-Hz

  k_printf("\n\nClock initialized: %dHz\n", clockFreq);
  
  asm volatile("sti");
  init_keyboard(); // Initialise keyboard
  
  asm volatile("sti");
  init_mouse(); // Initialise mouse

  k_printf("\nInitialized Keyboard\n\n");

  //~ char *eraseMe;
  //~ k_strcpy("My dad is a winner", eraseMe);
  //~ k_printf("\n%s\n", eraseMe);

  addShellIndent();
  beep();

  //~ asm volatile("sti");
  //~ asm volatile("mov  %ah, 0x00");
  //~ asm volatile("mov  %al, 0x13");
  //~ asm volatile("int $0x10");
  //~ while(1)
  //~ {
//~ 
  //~ }
  //~ while(1)
  //~ {
    //~ mSleep(100);
    //~ int number = getSystemUpTime();
    //~ k_printf("\n%d\n", number);
  //~ }

  /*Set video mode*/
  //~ VGA_init(320, 200, 256);

  //~ putPixel(100, 100, 4);
  //~ putRect(100, 100, 100, 100, 2);
  //~ putLine(60, 60, 120, 10, 6);

  /*go back to text mode*/
  //~ set_text_mode(0);
  
  return 0; //success!
}
