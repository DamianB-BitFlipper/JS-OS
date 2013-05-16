/*
 * ext2_fs.h
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


#ifndef FLOPPY_FS
#define FLOPPY_FS

///*FLOPPY DISK LAYOUT SCHEME
///* Block		Offset		Length						Description
// * byte 		0					512 							bytes	boot record (if present)
// * byte			512				512							  bytes	additional boot record data (if present)
// *
// * byte			1024			1024 							bytes	superblock
// * block 		2					1 block						block group descriptor table
// * block 		3					1 block						block bitmap
// * block 		4					1 block						inode bitmap
// * block	  5					23 blocks					inode table
// * block	  28				1412 blocks				data blocks

///Defaults
#define EXT2_BLOCK_SZ        1024 //a default block size of 1024 bytes
#define EXT2_I_PER_GRP       184  //derived from 23 blocks for inode table (23 * 1024 bytes) / (sizeof(ext2_inode) (128 bytes))

///Superblock offset
#define EXT2_SBLOCK_OFF      1024 //Superblock is always 1024 bytes offset, regardles of boot data is present

///Constants relative to the data blocks
#define EXT2_NDIR_BLOCKS     12                      //direct blocks
#define EXT2_IND_BLOCK       EXT2_NDIR_BLOCKS        //singly indirect blocks
#define EXT2_DIND_BLOCK      (EXT2_IND_BLOCK + 1)    //doubly indirect blocks
#define EXT2_TIND_BLOCK      (EXT2_DIND_BLOCK + 1)   //triply indirect blocks
#define EXT2_N_BLOCKS        (EXT2_TIND_BLOCK + 1)   //total number of blocks

///Inode mode values
enum EXT2_IMODE
{
  //-- file type --
  EXT2_I_SOCK = 0xC000,   //socket
  EXT2_I_SLNK = 0xA00,    //symbolic link
  EXT2_I_FILE = 0x800,    //regular file
  EXT2_I_BLK = 0x600,     //block device
  EXT2_I_DIR = 0x400,     //directory
  EXT2_I_CHR = 0x200,     //character device
  EXT2_I_FIFO = 0x100,    //fifo
  
  //-- process execution user/group override --
  EXT2_I_UID = 0x080,     //Set process User ID
  EXT2_I_GID = 0x040,     //Set process Group ID
  EXT2_I_SBIT = 0x020,    //sticky bit
  
  //-- access rights --
  EXT2_I_RUSR = 0x010,    //user read
  EXT2_I_WUSR = 0x008,    //user write
  EXT2_I_XUSR = 0x004,    //user execute
  EXT2_I_RGRP = 0x002,    //group read
  EXT2_I_WGRP = 0x001,    //group write
  EXT2_I_XGRP = 0x0008,   //group execute
  EXT2_I_ROTH = 0x0004,   //others read
  EXT2_I_WOTH = 0x0002,   //others write
  EXT2_I_XOTH = 0x0001    //others execute  
};

///Root privilege access right values, all of the rights combined
enum EXT2_ROOT_ACCESS_RIGHTS
{
	EXT2_RROOT = EXT2_I_RUSR | EXT2_I_RGRP | EXT2_I_ROTH,
	EXT2_WROOT = EXT2_I_WUSR | EXT2_I_WGRP | EXT2_I_WOTH,
	EXT2_XROOT = EXT2_I_XUSR | EXT2_I_XGRP | EXT2_I_XOTH
};

///Dirent type values
enum EXT2_DIRENT
{
  EXT2_UNKOWN,
  EXT2_FILE,
  EXT2_DIR,
  EXT2_CHARD_DEV,
  EXT2_BLOCK_DEV,
  EXT2_FIFO,
  EXT2_SOCKET,
  EXT2_SYMLINK
};

typedef struct ext2_superblock
{
  u32int total_inodes;
  u32int total_blocks;
  u32int reserved_blocks;
  u32int free_blocks;
  u32int free_inodes;
  u32int first_data_block;
  u32int log2_block_size;
  u32int log2_fragment_size;
  u32int blocks_per_group;
  u32int fragments_per_group;
  u32int inodes_per_group;
  u32int mtime;
  u32int utime;
  u16int mnt_count;
  u16int max_mnt_count;
  u16int magic;
  u16int fs_state;
  u16int error_handling;
  u16int minor_revision_level;
  u32int lastcheck;
  u32int checkinterval;
  u32int creator_os;
  u32int revision_level;
  u16int uid_reserved;
  u16int gid_reserved;
  u32int first_inode;
  u16int inode_size;
  u16int block_group_number;
  u32int feature_compatibility;
  u32int feature_incompat;
  u32int feature_ro_compat;
  u32int unique_id[4];
  char volume_name[16];
  char last_mounted_on[64];
  u32int compression_info;
}ext2_superblock_t;

typedef struct ext2_group_descriptor
{
  u32int block_bitmap;
  u32int inode_bitmap;
  u32int inode_table_id;
  u16int free_blocks;
  u16int free_inodes;
  u16int used_dirs;
  u16int pad;
  u32int reserved[3];  
}ext2_group_descriptor_t;

typedef struct ext2_inode
{
  u16int mode;
  u16int uid;
  u16int size;
  u16int atime;
  u16int ctime;
  u16int mtime;
  u16int dtime;
  u16int gid;
  u16int nlinks;
  u16int nblocks; /* Blocks of 512 bytes!! */
  u16int flags;
  u16int osd1;
  u32int blocks[EXT2_N_BLOCKS];
  u16int version;
  u16int fire_acl;
  u16int dir_acl;
  u16int fragment_addr;
  u16int osd2[3];
}ext2_inode_t;

struct ext2_dirent
{
  u32int ino;               //inode number. Required by POSIX.
  u16int rec_len;           //bytes from begining of this dirent to end of this dirent, size of file
  u8int name_len;           //the number of bytes of charachters in the name
  u8int file_type;          //a flag that states what type of file this dirent is (ie: a file, pipe, directory, etc.)
  char *name;               //filename, remember to kmalloc this to give it an address, or else it will page fault
};

/*set the block group of a floppy*/
u32int ext2_set_block_group(u32int group_number);

#endif
