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

typedef struct fs_node
{
  u8int magic;
  char name[128];
  u32int mask;              //the permissions mask.
  u32int uid;               //the owning user.
  u32int gid;               //the owning group.
  u32int flags;             //includes the node type (file, directory, etc.). See #defines above.
  u32int inode;             //this is device-specific - provides a way for a filesystem to identify files.
  u32int length;            //size of the file, in bytes.
  u32int impl;              //an implementation-defined number.

  u32int blocks[12];        //Data blocks (1 KB each)
  
  u32int *singly;
  u32int *doubly;
  u32int *triply;

  read_type_t read;         //function to file read event
  write_type_t write;       //function to file write event
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

typedef u8int* (*blk_dev_write)(u32int *data, u32int size, u32int sectorLBA);
typedef u8int* (*blk_dev_read)(u32int sectorLBA, u32int size, u32int *output);
typedef u32int (*blk_dev_change_drive)(u8int drive);

typedef struct vfs_blkdev
{
  u32int lock;
  u32int drive;
  u32int fs_type;
  s32int inode; //inode number of the block device file
  blk_dev_write write;
  blk_dev_read read;
  blk_dev_change_drive change_drive;
} vfs_blkdev_t;

extern fs_node_t *fs_root; // The root of the filesystem.

// Standard read/write/open/close functions. Note that these are all suffixed with
// _fs to distinguish them from the read/write/open/close which deal with file descriptors
// , not file nodes.
u32int read_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer);
u32int write_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer);

/*fs_node_t *node is the directory node to search in, returns dirent of file at the index input*/
struct dirent *readdir_fs(fs_node_t *node, u32int index);

/*fs_node_t *node is the directory node to search in, returns fs_node_t of file the the name input*/
fs_node_t *finddir_fs(fs_node_t *node, char *name);

/*creates a directory in the ramdisk filesystem*/
fs_node_t *vfs_createDirectory(fs_node_t *parentNode, char *name);

/*creates a file in the ramdisk filesystem*/
fs_node_t *vfs_createFile(fs_node_t *parentNode, char *name, u32int size);

/*moves a file to a directory*/
int addFileToDir(fs_node_t *dirNode, fs_node_t *fileNode);

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

/*removes a dirent from a vfs directory*/
u32int vfs_remove_dirent(fs_node_t *directory, fs_node_t *node);

/*free the data blocks of a node*/
u32int vfs_free_data_blocks(fs_node_t *directory, fs_node_t *node);

/*reads the data from a block device file*/
vfs_blkdev_t *vfs_read_blkdev(u32int inode);

/*register a block device*/
fs_node_t *vfs_register_blkdev(char *name, vfs_blkdev_t *data);

/*update the input blk device data block, the inode of the file to update is in the typedef of vfs_blkdev_t*/
u32int vfs_change_blkdev(vfs_blkdev_t *data);

#endif
