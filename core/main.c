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

u32int main(struct multiboot *mboot_ptr, u32int initial_stack)
{
  u32int clockFreq = 1000; //initialized clock to 1000 ticks per second

  // Initialise the screen (by clearing it)
  k_clear();

  setScreenYMinMax(1, 25); //reserve 1 row at the top for OS name

  k_setprintf(0, 0, "%Cw%cbk  %s %s                                                                    %Cbk%cw ", OS_NAME, OS_VERSION, 0);

  //possible non-stable release warning
  k_warning("Warning!!! This is NOT a stable release, Latest stable release is version:", OS_VERSION, 2);
  k_warning(" Latest stable release ID is: ", "12621d1855a0e332600fe7e79d12993ccf248a27          ", 3);

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

  // Initialise the initial ramdisk, and set it as the filesystem root.
  fs_root = initialise_initrd(initrd_location);

  //if there is some error in the vfs, it will be fatal!
  if(!fs_root)
  {
    k_printf("Error: VFS not initialized properly, fatal error");

    //stop any interupts
    asm volatile("cli");

    //freeze the OS
    for(;;);

    return 1; //error!, and also very bad :(
  }

  k_printf("Initialized the VFS file system!\n");

  ///Create a few test files and directories
  void *testDir = f_make_dir(fs_root, "direct");
  FILE *testFile = f_open("test_file", testDir, "wd");

  void *testDir2 = f_make_dir(testDir, "awesome");

  //test if the created file has been opened properly
  if(testFile)
  {

    f_write(testFile, 0, strlen("\t\"test_file\" read successfully, VFS is working\n"),
            "\t\"test_file\" read successfully, VFS is working\n");

    //after writing, close the file
    f_close(testFile);

    program_cat("/direct/test_file");
  }else
    k_printf("Error: test_file cannot be opened properly\n");


  // Initialise the floppy disk controller
  init_floppy();

  //initialize the ext2 driver on the floppy disk
  if(ext2_initialize(FLOPPY_SIZE - EXT2_SBLOCK_OFF, "/dev/fdb"))
    k_printf("%crFailed to Initialize the EXT2 file system%cw\n");
  else
    k_printf("Initialized the EXT2 file system!\n");

  datetime_t n = getDatetime();
  k_printf("The CMOS time is:\n\t%d:%d:%d %d/%d/%d\n",n.hour, n.min, n.sec, n.month, n.day, n.year);

  //greet the user
  greeting_message();
  
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
    //~ start_task(PRIO_HIGH, PROC_SHORT, tasking_test, "tasking", "tasking_test");

    s32int pid = fork(PRIO_LOW, PROC_SHORT, "test");

    k_printf("Testing multitasking...\n");

    if(!pid)
    {
      k_printf("\tMultitasking is operational with task PID: %h\n", getpid());
      exit();
    }

    //we have no error since if pid never equals 0, then something with assert, page fault etc.

    //sucess
    return 0;
  }

  //something went wrong
  return 1;
}
