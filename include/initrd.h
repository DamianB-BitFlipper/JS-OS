/*
 * initrd.h
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

#ifndef INITRD_H
#define INITRD_H

#include <system.h>

#define FS_SIZE                       sizeof(fs_node_t) * 0x4000
#define BLOCK_SIZE                    1024
#define DIRECTORY_SIZE                BLOCK_SIZE //the size of the directory (bytes)

extern fs_node_t *root_nodes;              // List of file nodes.

typedef struct
{
  u32int nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct
{
  u8int magic;      // Magic number, for error checking.
  s8int name[128];  // Filename.
  u32int offset;    // Offset in the initrd that the file starts.
  u32int length;    // Length of the file.
} initrd_file_header_t;

//set the initial file desctripter to be globally gloabal :)
extern file_desc_t *initial_fdesc;

/*Initialises the initial ramdisk. It gets passed the address of the multiboot module,
 * and returns a completed filesystem node */
fs_node_t *initialise_initrd(u32int location);

/*returns a dirent at the location of index (NOTE: the index does
 * not have to equal the inode of that dirent) */
struct dirent *initrd_readdir(fs_node_t *dirNode, u32int index);

/*returns the fs_node_t information of a file in the directory dirNode
 * with the filename of char *name */
fs_node_t *initrd_finddir(fs_node_t *dirNode, char *name);

/*reads from file*/
u32int initrd_read(fs_node_t *node, u32int offset, u32int size, u8int *buffer);

/*writes to a file*/
u32int initrd_write(fs_node_t *node, u32int offset, u32int size, u8int *buffer);

#endif
