/*
 * main.c
 *
 * Copyright 2013 JS <js@duck-squirell>
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


//~ #include "monitor.h"
#include "paging.h"
#include "task.h"
#include "syscall.h"

//Basics
#include "k_stdio.h"
#include "descriptor_tables.h"
#include "timer.h"

//IO
#include "keyboard.h"
#include "mouse.h"
#include "sound.h"

//Filesystem
#include "fs.h"
#include "initrd.h"
//~ #include "isr.h"
#include "multiboot.h"

//User interface
#include "k_shell.h"
#include "graphics.h"

//Other
#include "k_programs.h"


#include "vesa.h"


//~ #include "vga_modes.h"

extern u32int placement_address;
u32int initial_esp;

u32int initrd_location;
u32int initrd_end;

extern initrd_file_header_t *file_headers; // The list of file headers.


int main(struct multiboot *mboot_ptr, u32int initial_stack)
{
  int clockFreq = 1000; //initialized clock to 1000 ticks per second

  initial_esp = initial_stack;
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
  // Initialise the screen (by clearing it)
  k_clear();

  //~ asm volatile("sti");
  //~ asm volatile("int $0x3");

  /**ADDED**/
  setScreenYMinMax(1, 25); //reserve 1 row at the top for OS name

  k_setprintf(0, 0, "%Cw%cbk  JS-OS 0.0.1                                                                    %Cbk%cw ");

  k_printf("Hello World");
  k_printf("\nWelcome to JS-OS, Kernel booted. Running OS.\tAnd thus i poop\n");

  //~ asm volatile("int $0x3");
  //~ asm volatile("int $0x4");

  // Initialise the PIT to clockFreq-Hz
  asm volatile("sti");
  init_timer(clockFreq);

  asm volatile("sti");
  init_timer(clockFreq); // Initialise timer to clockFreq-Hz

  k_printf("\nClock initialized: %dHz\n", clockFreq);

  asm volatile("sti");
  init_keyboard(); // Initialise keyboard

  asm volatile("sti");
  init_mouse(); // Initialise mouse

  k_printf("\nInitialized Keyboard and Mouse\n\n");

  // Find the location of our initial ramdisk.
  ASSERT(mboot_ptr->mods_count > 0);
  initrd_location = *((u32int*)mboot_ptr->mods_addr);
  initrd_end = *(u32int*)(mboot_ptr->mods_addr+4);
  // Don't trample our module with placement accesses, please!
  placement_address = initrd_end;

  //~ k_printf("\ninitrd_location: %h, initrd_end: %h, placement_address: %h", initrd_location, initrd_end, placement_address);
  //~ while(1);.

  u32int memorySize = ((mboot_ptr->mem_lower + mboot_ptr->mem_upper) * 1024); //Bytes

  k_printf("size of memory: %d, %h", memorySize, memorySize);

  // Start paging.
  initialise_paging(memorySize);
  //~ initialise_paging(0x1000000);

  // Start multitasking.
  initialise_tasking();

  k_printf("\nPOOP");

  // Initialise the initial ramdisk, and set it as the filesystem root.
  fs_root = initialise_initrd(initrd_location);

  k_printf("\n");
  //~ k_printf_hex(initrd_location);
  k_printf("\n");

  /**Also part of the multi-task**/

  // Create a new process in a new address space which is a clone of this.
  //~ int ret = fork();
//~
  //~ k_printf("\nPOOP2");
//~
  //~ k_printf("fork() returned ");
  //~ k_printf("%h", ret);
  //~ k_printf(", and getpid() returned ");
  //~ k_printf("%h", getpid());
  //~ k_printf("\n============================================================================\n");
//~
  //~ //// The next section of code is not reentrant so make sure we aren't interrupted during.
  //~ asm volatile("cli");
  //// list the contents of /
  /**Also part of the multi-task**/

  //k_printf("\noffset: %d", blockSizeAtIndex(4000, 0, 2500));
  //k_printf("\noffset: %d", blockSizeAtIndex(4000, 1, 2500));
  //k_printf("\noffset: %d", blockSizeAtIndex(4000, 2, 2500));
  //k_printf("\noffset: %d\n", blockSizeAtIndex(4000, 3, 2500));

  ///*FILE SYSTEM*/
  int i = 0;
  struct dirent *node = 0;

  fs_node_t *testDir = createDirectory(fs_root, "direct");
  fs_node_t *testFile = createFile(testDir, "poop", 9);

  fs_node_t *testDir2 = createDirectory(testDir, "awesome");


  write_fs(testFile, 0, 9, "Hi World!");

  //~ setCurrentDir(testDir2);

  //~ node = readdir_fs(fs_root, 0);
  //~ k_printf("\nName: %s , inode is: %d, file_header: %s\n", node->name, node->ino, file_headers[2].name);
  //~ addFileToDir(testDir, testFile);
  //~ addFileToDir(testDir, testFile);

  //FIXME add file to dir does not work properly

  while ( (node = readdir_fs(fs_root, i)) != 0)
  {
    k_printf("Found file ");
    k_printf(node->name);
    fs_node_t *fsnode = finddir_fs(fs_root, node->name);

    if ((fsnode->flags&0x7) == FS_DIRECTORY)
    {
      k_printf("\n\t(directory)\n");
    }
    else
    {
      k_printf("\n\t contents: \"");
      unsigned char buf[256];
      //~ unsigned char writeBuf[9] = "Hi World!";

      //~ write_fs(fsnode, 0, 9, writeBuf);

      u32int sz = read_fs(fsnode, 0, 256, buf);
      int j;
      for (j = 0; j < sz; j++)
      {
        k_putChar(buf[j]);
        //~ k_printf(" %h", buf[j]);
      }

      k_printf("\"\n");
    }
    i++;
  }
  ///*FILE SYSTEM*/

  asm volatile("sti");

  addShellIndent();

  //char *test;
  //char row[3] = "do";
  //*(row + 2) = 0;

  //test = (char*)kmalloc(25);

  //memcpy(test + 22, row, 3);

  //shiftData((test + 22), -22, 3);

  //~ k_printf("\n\n\nTEST IS: %s", test);

  //~ setVesa(0x118);

  //~ kmalloc(1024*768*4);

  //~ set_vga_mode(VESA_MODE_824);
  //~ set_vga_mode(VGA_MODE_13h);

  /**Also part of the multi-task**/
  //k_printf("\n");
  //asm volatile("sti");
  /**Also part of the multi-task**/

  //~ initialise_syscalls();
//~
  //~ switch_to_user_mode();
//~
  //~ syscall_user_printf("Hello, user world! PPOOOOPP\n");
  //~ syscall_user_putChar('G');

  //~ asm volatile("sti");
  //~ init_timer(clockFreq); // Initialise timer to clockFreq-Hz

  //~ while(1)
  //~ {
    //~ syscall_user_putChar('G');
    //~ mSleep(500);
  //~ }

  return 0;
}
