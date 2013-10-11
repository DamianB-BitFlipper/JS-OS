/*
 * std_fs.h
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

#ifndef STD_FS_H
#define STD_FS_H

//for the file descriptor
#define FDESC_READ    0b100
#define FDESC_WRITE   0b10
#define FDESC_APPEND  0b1

//define the magic numbers for the file systems
#define M_UNKNOWN       0
#define M_VFS           1
#define M_EXT2          2

char *path;                  //path name of the directories
void *ptr_currentDir;        //pointer to the current directory

enum
{
  TYPE_UNKOWN,
  TYPE_FILE,
  TYPE_DIRECTORY,
  TYPE_CHARD_DEV,
  TYPE_BLOCK_DEV,
  TYPE_FIFO,
  TYPE_SOCKET,
  TYPE_SYMLINK,
  TYPE_HARDLINK,
  TYPE_MOUNTPOINT

};

/*for the file desctptor*/
typedef struct file_desc
{
  u8int permisions;
  u8int fs_type;
  void *node;
  struct file_desc *next;
} file_desc_t;

//create the typedef for a FILE
typedef file_desc_t FILE;

//set the initial file desctripter to be globally gloabal :)
extern file_desc_t *initial_fdesc;

/*generic set the current directory*/
u32int setCurrentDir(void *node);

/*formats the mask for the file descriptor of an open file*/
u8int __open_fs_mask_to_u32int__(char *mask);

/*obtains the file descriptor of a certain open node*/
file_desc_t *look_up_fdesc(void *node);

// Standard read/write/open/close functions. Note that these are all suffixed with
// _fs to distinguish them from the read/write/open/close which deal with file descriptors
// , not file nodes.
u32int read_fs(void *node, u32int offset, u32int size, u8int *buffer);
u32int write_fs(void *node, u32int offset, u32int size, u8int *buffer);
FILE *open_fs(char *filename, void *dir, char *mask);
u32int close_fs(FILE *file);

/*fs_node_t *node is the directory node to search in, returns dirent of file at the index input*/
void *readdir_fs(void *node, u32int index);

/*fs_node_t *node is the directory node to search in, returns fs_node_t of file the the name input*/
void *finddir_fs(void *node, char *name);

/*returns the type of the file, regardless what type it is*/
u32int type_of_node(void *node);

/*get the name of a node*/
char *name_of_node(void *parent_dir, void *node);

#endif //STD_FS_H
