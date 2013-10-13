/*
 * vfs.h
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

#ifndef FS_H
#define FS_H

#include <system.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?

struct fs_node;

// These typedefs define the type of callbacks - called when read/write/open/close
// are called.
typedef u32int (*read_type_t)(struct fs_node*,u32int,u32int,u8int*);
typedef u32int (*write_type_t)(struct fs_node*,u32int,u32int,u8int*);
//~ typedef void (*open_type_t)(struct fs_node*);
//~ typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent *(*readdir_type_t)(struct fs_node*, u32int);
typedef struct fs_node *(*finddir_type_t)(struct fs_node*, char *name);

//define the number of blocks per each level
#define BLOCKS_DIRECT     12          //12 KB
#define BLOCKS_SINGLY     256         //256 KB
#define BLOCKS_DOUBLY     65536       //65536 (256 * 256) KB (64 MB)
#define BLOCKS_TRIPLY     16777216    //16777216 (256 * 256 * 256) KB (16 GB)

//~ typedef struct fs_singly
//~ {
  //~ u32int blocks[256];
//~ } fs_singly_t;
//~ 
//~ typedef struct fs_doubly
//~ {
  //~ fs_singly_t singly[256];
//~ } fs_doubly_t;
//~ 
//~ typedef struct fs_triply
//~ {
  //~ fs_doubly_t doubly[256];
//~ } fs_triply_t;

typedef struct fs_node
{
  char name[128];
  u32int mask;              //the permissions mask.
  u32int uid;               //the owning user.
  u32int gid;               //the owning group.
  u32int flags;             //includes the node type. See #defines above.
  u32int inode;             //this is device-specific - provides a way for a filesystem to identify files.
  u32int length;            //size of the file, in bytes.
  u32int impl;              //an implementation-defined number.

  u32int blocks[12];        //Data blocks (1 KB each)
  //~ fs_singly_t *singly;      //pointer to a singly indirect data set
  //~ fs_doubly_t *doubly;      //pointer to a doubly indirect data set
  //~ fs_triply_t *triply;      //pointer to a triply indirect data set
  
  u32int *singly;
  u32int *doubly;
  u32int *triply;

  read_type_t read;         //function to file read event
  write_type_t write;       //function to file write event
  //~ open_type_t open;         //function to file open event
  //~ close_type_t close;       //function to file close event
  readdir_type_t readdir;   //function to file readdir event
  finddir_type_t finddir;   //function to file finddir event
  struct fs_node *ptr;      //used by mountpoints and symlinks.
}fs_node_t;

struct dirent
{
  u32int ino;               //inode number. Required by POSIX.
  u16int rec_len;           //bytes from begining of this dirent to end of this dirent, size of dirent
  u8int name_len;           //the number of bytes of charachters in the name
  u8int file_type;          //a flag that states what type of file this dirent is (ie: a file, pipe, directory, etc.)
  char *name;               //filename, remember to kmalloc this to give it an address, or else it will page fault
};

extern fs_node_t *fs_root; // The root of the filesystem.

// Standard read/write/open/close functions. Note that these are all suffixed with
// _fs to distinguish them from the read/write/open/close which deal with file descriptors
// , not file nodes.
u32int read_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer);
u32int write_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer);
FILE *open_fs(char *filename, fs_node_t *dir, char *mask);
u32int close_fs(FILE *file);

/*fs_node_t *node is the directory node to search in, returns dirent of file at the index input*/
struct dirent *readdir_fs(fs_node_t *node, u32int index);

/*fs_node_t *node is the directory node to search in, returns fs_node_t of file the the name input*/
fs_node_t *finddir_fs(fs_node_t *node, char *name);

/*creates a directory in the ramdisk filesystem*/
fs_node_t *createDirectory(fs_node_t *parentNode, char *name);

/*creates a file in the ramdisk filesystem*/
fs_node_t *createFile(fs_node_t *parentNode, char *name, u32int size);

/*moves a file to a directory*/
int addFileToDir(fs_node_t *dirNode, fs_node_t *fileNode);

/*sets the current directory to a input directory and gets sets the path char array*/
int setCurrentDir(fs_node_t *directory);

/*returns the value of an open inode value for a file, directory, etc. */
int findOpenNode();

/*funtion returns 0 on success and 1 on failure, shift the data siftAmount units
 * If shiftAmount < 0, shifts to the left, and shiftAmount > 0, shifts to the right */
int shiftData(void *position, int shiftAmount, u32int lengthOfDataToShift);

/*with a given fileSize and block number, it will return how many bytes
 * the file is using in that block number input */
u32int blockSizeAtIndex(u32int fileSize, u32int blockNum, u32int offset);

/*returns the pointer to the correct block in the block hierarchy of a file node*/
u32int *block_of_set(fs_node_t *node, u32int block_number);

/*looks up and returns a file descriptor if it exitsts*/
file_desc_t *look_up_fdesc(fs_node_t *node);

#endif
