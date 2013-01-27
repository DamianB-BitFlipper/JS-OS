// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "k_stdio.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "keyboard.h"

#include "k_shell.h"

struct multiboot *mboot_ptr;

int main()
{
  int clockFreq = 50; //initialized clock to 50Hz
  
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
  
  // Initialise the screen (by clearing it)
  k_clear();
  // Write out a sample string
  //~ monitor_write("Hello, world----!");
  //~ k_printf("Heeeeello, wor%dld %d ----333", 5000, 360);
  //~ k_printf(
  k_printf("Hello World");
  k_printf("\nWelcome to JS-OS, Kernel booted. Running OS.\tAnd thus i poop\n");

  asm volatile("int $0x3");
  asm volatile("int $0x4");

  asm volatile("sti");
  init_timer(clockFreq); // Initialise timer to clockFreq-Hz

  k_printf("\n\nClock initialized: %dHz\n", clockFreq);
  
  asm volatile("sti");
  init_keyboard(); // Initialise keyboard

  k_printf("\nInitialized Keyboard\n\n");

  //~ char *eraseMe;
  //~ k_strcpy("My dad is a winner", eraseMe);
  //~ k_printf("\n%s\n", eraseMe);

  addShellIndent();

    
  return 0; //success!
}
