/*
 * elf.c
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

// Based on code from CakeOS

#include <system.h>

u32int relocModule_elf(elfHeader *header, u32int offset)
{
  /*The argument u32int offset is the offset (memory location) in the OS
   * as to where the binary information should be stored */

  u32int i;
  static programHeader* pHeader;
  //static noteEntry* note;
  
  k_printf("[relocModule_elf] %d Program Headers found, phentsize %d.\n", header->phnum, header->phentsize);
  
  for(i = 0; i < header->phnum; i++)
  {
    pHeader = (programHeader*)((unsigned long) header + header->phoff + i * header->phentsize);
    
    /* Check if its loadable */
    switch(pHeader->type)
    {
     
      case PT_LOAD:
        k_printf("[relocModule_elf] Program header #%d Type:LOAD Offset:%h Vaddr:%h, Paddr:%h, Size:%h, MEM:%h, Align:%h!\n", i, pHeader->offset, pHeader->vaddr, pHeader->paddr, pHeader->fileSize, pHeader->memSize, pHeader->alignment);

        //~ page_t *page;
//~ 
        //~ u32int j = pHeader->vaddr & ~0xFFF;
        //~ u32int limit = ((u32int) pHeader->vaddr + pHeader->memSize + 0x1000) & ~0xFFF;

        //Map the virtual address to an area in the stack
        //~ for(;j < limit; j+= 0x1000)
        //~ {
          //~ page = get_page(pHeader->vaddr, 1, current_directory);
          //~ alloc_frame(page, 0, 1);
          //~ k_printf("frame:  %h\n", page->frame);
        //~ }
        
        //Map the virtual address to an area in the stack
        virtual_map_pages(pHeader->vaddr, pHeader->fileSize, TRUE, TRUE);

        k_printf("[relocModule_elf] pHeader->vaddr source: %h\n", offset + pHeader->offset);

        //~ memcpy((u32int*)pHeader->vaddr, (const void*)offset + pHeader->offset, pHeader->memSize);
        
        ///* Copy the program data to vaddr */
        memcpy((char*)pHeader->vaddr, (char*)offset + pHeader->offset, pHeader->fileSize);
        //~ memcpy((char*)pHeader->vaddr, (char*)offset + pHeader->offset, pHeader->memSize);
        if(pHeader->fileSize < pHeader->memSize)
        {
          /* memset() the remaining memory with zeroes - stack */
          memset((char*)pHeader->vaddr + pHeader->fileSize, 0, pHeader->memSize - pHeader->fileSize);
        }
        
        break;
      
      case PT_DYNAMIC:
        k_printf("[relocModule_elf] Program header #%d Type:DYNAMIC Addr:%h Size:%h!\n", i, pHeader->vaddr, pHeader->fileSize);
        //I failed to understand what needs to be done here
        break;
  
      case PT_INTERP:
        k_printf("[relocModule_elf] Program header #%d Type:INTERP Addr:%h Size:%h!\n", i, pHeader->vaddr, pHeader->fileSize);
        
        //This is an external program that needs to be relocated and called.
        static unsigned char *interp;
        
        memcpy((char*)interp, (char*)offset + pHeader->offset, pHeader->fileSize);
        
        k_printf("\tInterpreter: \"%s\"\n", interp);
        break;
      
      case PT_NOTE:
        k_printf("[relocModule_elf] Program header #%d Type:NOTE Addr:%h Size:%h!\n", i, pHeader->vaddr, pHeader->fileSize);
      
        //~ debug_noteEntries((u32int*)(offset+pHeader->offset));
        break;
      
      //case PT_SHLIB:
      //break;
      
      //case PT_PHDR:
      //break;
      
      default:
      break;
    }       
  }
  return 1;
}


u32int openModule_elf(char *name, u32int offset)
{
  k_printf("[openModule_elf] Opening %s (ELF module)\n",name);
  
  elfHeader *header;
  
  /* Put header information into header struct */
  header = (elfHeader*)offset;
  
  /* Check for any errors */
  if(header->ident[0] != 0x7f || header->ident[1] != 'E' || header->ident[2] != 'L' || header->ident[3] != 'F')
  {
    k_printf("[openModule_elf] Error: not a valid ELF binary\n");
    return 1;
  }
  
  /* Bits */
  if(header->ident[4] != 1) //1 = 32, 2 = 64
  { 
    k_printf("[openModule_elf] Error: wrong architecture bit number\n");
    return 1;
  }
  
  /* Endian type */
  if(header->ident[5] != 1) //1 = little, 2 = big
  {
    k_printf("[openModule_elf] Error: wrong architecture endianess\n");
    return 1;
  }
  
  //~ debug_sectionHeaders(header);
  
  if(relocModule_elf(header, offset) != 1)
  {
    k_printf("[openModule_elf] Error: relocation failed\n");
    return 1;
  }
  
  k_printf("[openModule_elf] Calling executable at %h\n", header->entry);
  
  /* Call the program */
  for(;;);
  //~ asm volatile("call *%0" : : "a" (offset));
  asm volatile("call *%0" : : "a" (header->entry));
  //~ asm volatile("call *%0" : : "a" (header));
  //~ execute_binary(header->entry);
  //~ start_task(PRIO_LOW, PROC_SHORT, (void*)header->entry, 0, "try");
  
  return 0;
}

u32int load_elf(u32int inode, u32int size)
{
  k_printf("[load_elf] Called for inode %d - %d bytes of data allocated\n", inode, size);

  char *elf_name = root_nodes[inode].name;
  
  //tests if this module exist in the root
  if(finddir_fs(fs_root, elf_name) == 0)
  {
    return 1;
  }else{ //the module exists
    k_printf("[load_elf] Directory entry read correctly\n");
    k_printf("[load_elf] Looking for \"%s\"\n", elf_name);

    //get the node information from the dirent
    fs_node_t *modulenode = finddir_fs(fs_root, elf_name);
    
    k_printf("[load_elf] Reading file\n");

    //create the actuall module buffer 
    char *modulebuffer = (char*)kmalloc(size);
    u32int modulesize = read_fs(modulenode, 0, size, modulebuffer);
    
    k_printf("[load_elf] Allocating space\n");

    //create a pointer and copy the above saved buffer that address
    u32int moduleptr = kmalloc(modulesize);
    memcpy((void*)moduleptr,(void*) modulebuffer, modulesize);
    
    k_printf("[load_elf] Data moved into allocated space: %h\n", moduleptr);
    
    if(openModule_elf(elf_name, moduleptr))
      k_printf("[load_elf] openModule_elf returned with an error\n");
  }
  return 0;
}

void execute_binary(u32int addr)
{
  asm volatile("call *%0" : : "a" (addr));
}
