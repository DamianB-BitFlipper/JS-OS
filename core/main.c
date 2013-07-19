/*
 * main.c
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

  k_setprintf(0, 0, "%Cw%cbk  %s %s                                                                    %Cbk%cw ", OS_NAME, OS_VERSION, 0);

  //possible non-stable release warning
  k_warning("Warning!!! This is NOT a stable release, Latest stable release is version:", OS_VERSION, 2);
  k_warning("  Latest stable release ID is: ", "18d732d838b30c73ffbc7bec5225c891058ea287         ", 3);

  k_printf("Hello World\n");
  k_printf("Welcome to %s, Kernel booted. Running OS.\n", OS_NAME);

  initial_esp = initial_stack;
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
  k_printf("IDT initialized\n");
  k_printf("GDT initialized\n");

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

  //the start of the initrd
  initrd_location = *((u32int*)mboot_ptr->mods_addr);
  //the end of the initrd is read 4 bytes after the start
  initrd_end = *(u32int*)(mboot_ptr->mods_addr + 4);

  // Don't trample our module with placement accesses, please!
  placement_address = initrd_end;

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

  // Initialise the floppy disk controller
  init_floppy();

  //test the FDC
  //~ test("floppy disk controller");

  // Initialise the initial ramdisk, and set it as the filesystem root.
  fs_root = initialise_initrd(initrd_location);
  k_printf("Initialized the filesystem\n");

  ///Create a few test files and directories
  fs_node_t *testDir = createDirectory(fs_root, "direct");
  fs_node_t *testFile = createFile(testDir, "test_file", 42);

  fs_node_t *testDir2 = createDirectory(testDir, "awesome");

  write_fs(testFile, 0, 9, "Hello, the file \"test_file\" is being read\n");
  program_cat("./direct/test_file");

  datetime_t n = getDatetime();
  k_printf("The CMOS time is:\n\t%d:%d:%d %d/%d/%d\n",n.hour, n.min, n.sec, n.month, n.day, n.year);

  greeting_message();

  ext2_initialize(FLOPPY_SIZE - EXT2_SBLOCK_OFF);
  
  addShellIndent();

  //sucess!
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
    start_task(PRIO_HIGH, PROC_SHORT, tasking_test, "tasking", "tasking_test");

    //sucess
    return 0;
  }else if(!strcmp(test, "floppy disk controller"))
  {
    if(_FloppyStorage == FALSE) //persistent storage is not enabled
    {

      k_printf("Floppy test has no persistent storage enabled\n");

      //no error has occured, so return 0
      return 0;
    }

    //THIS IS A TEST FOR THE FLOPPY READ AND WRITE FUNCTIONS
    //!WARNING, watch what sector you are writing to, you could write over boot data
    //!WARNING, on the boot floppy, messing the grub/kernel/initrd on the floppy
    u8int *sector = 0;
    u32int sector_index = 0;
    u32int size_to_write = SECTOR_SIZE;
    u8int write_data = 0xff;
    u8int *mem;
    mem = (u8int*)kmalloc(size_to_write);
    memset(mem, write_data, size_to_write);

    floppy_write((u32int*)mem, size_to_write, sector_index);

    floppy_read(sector_index, size_to_write, (u32int*)sector);

    if(sector)
    {
      k_printf("\tFloppy test wrote %h to sector %d with size %d bytes\n", write_data, sector_index + 1, size_to_write);

      //sucess!
      return 0;
    }else
    {
      k_printf("\tFloppy test Read/Write is not working!\n");
      //error!
      return 1;
    }
  }

  //something went wrong
  return 1;
}
