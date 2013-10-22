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

//~ #include <system.h>

//for the file descriptor
#define FDESC_CLEAR   0b1000
#define FDESC_READ    0b100
#define FDESC_WRITE   0b10
#define FDESC_APPEND  0b1

//define the magic numbers for the file systems
#define M_UNKNOWN       0
#define M_VFS           1
#define M_EXT2          2

///node type values
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
  char *name;
  u32int name_len;
  void *node;
  u32int fs_type;     //is it VFS, EXT2, etc.
  u32int node_type;   //is it a file, directory, ect.
  u8int permisions;
  s8int inode;
  u32int size;

  //callback operations
  //~ u32int (*_close)(void*);
  u32int (*_read)(void*, u32int, u32int, u8int*);
  u32int (*_write)(void*, u32int, u32int, u8int*);
  u32int (*_finddir)(void*, char*);
  u32int (*_readdir)(void*, u32int);

  //pointing to the next structure of the list
  struct file_desc *next;
} file_desc_t;

//create the typedef for a FILE
typedef file_desc_t FILE;

typedef struct generic_dirent
{
  s32int ino;               //inode number. Required by POSIX. -1 if no inode is present
  u16int rec_len;           //bytes from begining of this dirent to end of this dirent, size of file
  u8int name_len;           //the number of bytes of charachters in the name
  u8int file_type;          //a flag that states what type of file this dirent is (ie: a file, pipe, directory, etc.)
  char *name;           //filename, remember to kmalloc this to give it an address, or else it will page fault
} generic_dirent_t;

//set the initial file desctripter to be globally gloabal :)
extern file_desc_t *initial_fdesc;
extern char *path;                  //path name of the directories
extern void *ptr_currentDir;        //pointer to the current directory
extern file_desc_t *initial_fdesc;  //initial file descriptor location

/*read information from any supported file system node*/
u32int f_read(FILE *node, u32int offset, u32int size, u8int *buffer);

/*write information to any supported file system node*/
u32int f_write(FILE *node, u32int offset, u32int size, u8int *buffer);

/*open a file to be ready readding or writing*/
FILE *f_open(char *filename, void *dir, char *mask);

/*close an opened file*/
u32int f_close(FILE *file);

/*get dirent data from a directory's data*/
generic_dirent_t *f_readdir(void *node, u32int index);

/*get a node from a directory by name*/
FILE *f_finddir(void *node, char *name);

/*returns the type of a node, is it a file, directory, etc.*/
u32int node_type(void *node);

/*returns the name of a directory with a given node*/
char *name_of_dir(void *node);

/*set the current directory*/
u32int setCurrentDir(void *directory);

/*looks up and returns a file descriptor if it exitsts*/
file_desc_t *look_up_fdesc(void *node);

/*close file descriptors opened with f_finddir*/
void f_finddir_close(FILE *node);

/*retrieve a file descriptor from a node, without adding it the file descriptor list*/
FILE *__open__(void *node, char *name, char *mask, u8int open);

/*with a file descriptor, return the block size of its corresponding file system*/
u32int block_size_of_node(FILE *node);

/*delete a dirent of a directory*/
u32int __remove_dirent__(void *dir, FILE *dirent_node);

/*deletes only a node's data contents*/
u32int __free_data__(void *dir, FILE *node);

/*delete a file by freeing its contents and removing its dirent*/
u32int f_remove(void *dir, FILE *node);

/*creates a directory*/
void *f_make_dir(void *dir, char *name);

#endif //STD_FS_H
