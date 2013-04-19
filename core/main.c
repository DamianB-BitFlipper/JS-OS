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

#include <system.h>

u32int test(char *test);

extern u32int placement_address;
u32int initial_esp;

u32int initrd_location;
u32int initrd_end;

int main(struct multiboot *mboot_ptr, u32int initial_stack)
{
  u32int clockFreq = 1000; //initialized clock to 1000 ticks per second

  // Initialise the screen (by clearing it)
  k_clear();

  setScreenYMinMax(1, 25); //reserve 1 row at the top for OS name

  k_setprintf(0, 0, "%Cw%cbk  JS-OS 0.0.1                                                                    %Cbk%cw ");

  k_printf("Hello World\n");
  k_printf("Welcome to JS-OS, Kernel booted. Running OS.\n");

  initial_esp = initial_stack;
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
  k_printf("IDT initialized\n");
  k_printf("GDT initialized\n");

  //~ asm volatile("sti");
  //~ asm volatile("int $0x3");


  //~ asm volatile("int $0x3");
  //~ asm volatile("int $0x4");

  // Initialise the PIT to clockFreq-Hz
  asm volatile("sti");
  init_timer(clockFreq);

  k_printf("Clock initialized: %dHz\n", clockFreq);

  asm volatile("sti");
  init_keyboard(); // Initialise keyboard
  k_printf("Initialized keyboard\n");

  asm volatile("sti");
  init_mouse(); // Initialise mouse
  k_printf("Initialized mouse\n");

  // Find the location of our initial ramdisk.
  ASSERT(mboot_ptr->mods_count > 0);
  initrd_location = *((u32int*)mboot_ptr->mods_addr);
  initrd_end = *(u32int*)(mboot_ptr->mods_addr+4);
  // Don't trample our module with placement accesses, please!
  placement_address = initrd_end;

  //~ k_printf("\ninitrd_location: %h, initrd_end: %h, placement_address: %h", initrd_location, initrd_end, placement_address);
  //~ while(1);.

  u32int memorySize = ((mboot_ptr->mem_lower + mboot_ptr->mem_upper) * 1024); //Bytes

  k_printf("Size of memory: %d KB, %d MB\n", memorySize / 1024, memorySize / (1024 * 1024));

  //// Start paging.
  initialise_paging(memorySize);
  k_printf("Initialized paging\n");

  //test the memory
  test("memory");

  //// Start multitasking.
  initialise_tasking();
  k_printf("Initialized multitasking with PID: %h\n", getpid());
  
  //test the multitasking
  test("tasking");
  
  // Initialise the initial ramdisk, and set it as the filesystem root.
  fs_root = initialise_initrd(initrd_location);
  k_printf("Initialized the filesystem\n");
    
  /////*FILE SYSTEM*///
  //u32int i = 0;
  //struct dirent *node = 0;

  ///Create a few test files and directories
  fs_node_t *testDir = createDirectory(fs_root, "direct");
  fs_node_t *testFile = createFile(testDir, "test_file", 42);

  fs_node_t *testDir2 = createDirectory(testDir, "awesome");

  write_fs(testFile, 0, 9, "Hello, the file \"test_file\" is being read\n");
  program_cat("./direct/test_file");
  ///Create a few test files and directories
  
  //while((node = readdir_fs(fs_root, i)) != 0)
  //{
    //k_printf("[%d] Found file: %s", node->ino, node->name);
    ////~ k_printf(node->name);
    //fs_node_t *fsnode = finddir_fs(fs_root, node->name);

    //if ((fsnode->flags&0x7) == FS_DIRECTORY)
    //{
      //k_printf("\n\t(directory)\n");
    //}
    //else
    //{
      //k_printf("\n\t contents: \"");
      //unsigned char buf[256];
      ////~ unsigned char writeBuf[9] = "Hi World!";

      ////~ write_fs(fsnode, 0, 9, writeBuf);

      //u32int sz = read_fs(fsnode, 0, 256, buf);
      //int j;
      //for (j = 0; j < sz; j++)
      //{
        //k_putChar(buf[j]);
        ////~ k_printf(" %h", buf[j]);
      //}

      //k_printf("\"\n");
    //}
    //i++;
  //}
  /////*FILE SYSTEM*/

  ///This does not work yet
  //~ load_elf(2, root_nodes[2].length);
  ///This does not work yet

  datetime_t n = getDatetime();
  k_printf("The CMOS time is:\n\t%d:%d:%d %d/%d/%d\n",n.hour, n.min, n.sec, n.month, n.day, n.year);

  greeting_message();

  addShellIndent();

  return 0;
}

/*returns 0 for pass and 1 for fail*/
u32int test(char *test)
{
  if(!strcmp(test, "memory"))
  {
    u32int addr, size = 128;
    addr = kmalloc(size);
    kfree((void*)addr);

    //if we have not failed so far, print a sucessful message
    k_printf("\tMemory management test passed, allocated %d bytes at %h\n", size, addr);

    //sucess
    return 0;
  }else if(!strcmp(test, "tasking"))
  {
    start_task(PRIO_LOW, PROC_SHORT, tasking_test, "tasking", "tasking_test");

    //sucess
    return 0;
  }

  //something went wrong
  return 1;
}
