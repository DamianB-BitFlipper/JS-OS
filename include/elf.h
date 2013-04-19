/*
 * elf.h
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

// Based on code from CakeOS

#ifndef ELF_H
#define ELF_H

/* programHeader defines */
/* type */
#define PT_NULL 0             /* Unused */
#define PT_LOAD 1             /* Loadable segment (fileSize bytes) */
#define PT_DYNAMIC 2          /* Dynamic segment */
#define PT_INTERP 3           /* Size and location of null-ended path to interpreter */
#define PT_NOTE 4             /* Note segment */
#define PT_SHLIB 5            /* Reserved */
#define PT_PHDR 6             /* Size and location of program header */
#define PT_HIPROC 0x7fffffff  /* Reserved */
#define PT_LOPROC 0x70000000  /* Reserved */
/* !programHeader defines */

struct elfhdr
{
  u8int ident[16];   /* Elf identification */
  u16int type;        /* Type of ELF - 2 for Exectuable, 3 for Lib */
  u16int machine;     /* Architecture - 7 for x86 */
  u32int version;     /* 0 for invalid, 1 for valid */
  u32int entry;       /* Entry point - Point EIP here */
  u32int phoff;       /* Address of programHeader struct */
  u32int shoff;       /* Address of sectionHeader */
  u32int flags;       /* Machine flags - 0 for x86 */
  u16int ehsize;      /* elfHeader size */
  u16int phentsize;   /* Entry size from programHeader */
  u16int phnum;       /* Number of entries in programHeader */
  u16int shentsize;   /* Entry size of sectionHeader */
  u16int shnum;       /* Number of entries in sectionHeader */
  u16int shstrndx;    /* sectionHeader string index */
} __attribute__((packed));

typedef struct elfhdr elfHeader;

struct prgmhdr
{
  u32int type;          /* This contains the segment type */
  u32int offset;        /* Beginning of program */
  u32int vaddr;         /* Virtual addressm to relocate to */
  u32int paddr;         /* Physical address of above - Ignore */
  u32int fileSize;      /* Size of program */
  u32int memSize;       /* Memory size of program */
  u32int flags;         /* Flags relevent to segment */
  u32int alignment;     /* Alignment - 0 or 1 for none, Positive for align to value */
                                 /* NOTE: vaddr should equal offset */
} __attribute__((packed));

typedef struct prgmhdr programHeader;

/*executes the binary at a given address*/
void execute_binary(u32int addr);

/*load an elf binary for execution*/
u32int load_elf(u32int inode, u32int size);

#endif //ELF_H
