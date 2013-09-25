/*
 * kheap.h
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

// kheap.h -- Interface for kernel heap functions, also provides
//            a placement malloc() for use before the heap is 
//            initialised.
//            Based on code from JamesM's kernel development tutorials.

#ifndef KHEAP_H
#define KHEAP_H

#define KHEAP_START         0x40000000
#define KHEAP_INITIAL_SIZE  0x1000000

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0xDEADBEEF
#define HEAP_MIN_SIZE     0x70000

#include <system.h>

/**
   Size information for a hole/block
**/
typedef struct
{
  u32int magic;   // Magic number, used for error checking and identification.
  u8int is_hole;   // 1 if this is a hole. 0 if this is a block.
  u32int size;    // size of the block, including the end footer.
  
}header_t;

typedef struct
{
  u32int magic;     // Magic number, same as in header_t.
  header_t *header; // Pointer to the block header.
  
}footer_t;

typedef struct
{
  ordered_array_t index;
  u32int start_address; // The start of our allocated space.
  u32int end_address;   // The end of our allocated space. May be expanded up to max_address.
  u32int max_address;   // The maximum address the heap can be expanded to.
  u8int supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
  u8int readonly;       // Should extra pages requested by us be mapped as read-only?
  
}heap_t;


/**
   Create a new heap.
**/
heap_t *create_heap(u32int start, u32int end, u32int max, u8int supervisor, u8int readonly);

/**
   Allocates a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting
   on a page boundary.
**/
void *alloc(u32int size, u8int page_align, heap_t *heap);

/**
   Releases a block allocated with 'alloc'.
**/
void free(void *p, heap_t *heap);

/**
   Allocate a chunk of memory, sz in size. If align == 1,
   the chunk must be page-aligned. If phys != 0, the physical
   location of the allocated chunk will be stored into phys.

   This is the internal version of kmalloc. More user-friendly
   parameter representations are available in kmalloc, kmalloc_a,
   kmalloc_ap, kmalloc_p.
**/
u32int kmalloc_int(u32int sz, int align, u32int *phys);

/**
   Allocate a chunk of memory, sz in size. The chunk must be
   page aligned.
**/
u32int kmalloc_a(u32int sz);

/**
   Allocate a chunk of memory, sz in size. The physical address
   is returned in phys. Phys MUST be a valid pointer to u32int!
**/
u32int kmalloc_p(u32int sz, u32int *phys);

/**
   Allocate a chunk of memory, sz in size. The physical address 
   is returned in phys. It must be page-aligned.
**/
u32int kmalloc_ap(u32int sz, u32int *phys);

/**
   General allocation function.
**/
u32int kmalloc(u32int sz);

/**
   General deallocation function.
**/
void kfree(void *p);

/**
   Returns the amount of space the input pointer has alloced
**/
u32int size_of_alloc(void *alloc);

/*For reallocation of kmallocs, regular, a, p, ap*/
u32int krealloc(u32int *ptr, u32int old_sz, u32int new_sz);
u32int krealloc_a(u32int *ptr, u32int old_sz, u32int new_sz);
u32int krealloc_ap(u32int *ptr, u32int old_sz, u32int new_sz, u32int *phys);
u32int krealloc_p(u32int *ptr, u32int old_sz, u32int new_sz, u32int *phys);

#endif // KHEAP_H
