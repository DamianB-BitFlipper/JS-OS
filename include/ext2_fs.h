/*
 * ext2_fs.h
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


#ifndef FLOPPY_FS
#define FLOPPY_FS

// *          FLOPPY DISK LAYOUT SCHEME
// * Block    Offset    Length            Description
// * byte     0         512               bytes boot record (if present)
// * byte     512       512               bytes additional boot record data (if present)
// *
// * byte     1024      1024              bytes superblock
// * block    2         1 block           block group descriptor table
// * block    3         1 block           block bitmap
// * block    4         1 block           inode bitmap
// * block    5         23 blocks         inode table
// * block    28        1412 blocks       data blocks

///Defaults
#define EXT2_MAGIC           0xEF53
#define EXT2_BLOCK_SZ        1024         //a default block size of 1024 bytes
#define EXT2_MAX_BLK_GRP_SZ  8 * EXT2_BLOCK_SZ //maximum blocks in a block group, (8192) with block sizes of 1024
#define EXT2_MAX_INODES_GRP  8 * EXT2_BLOCK_SZ //maximum indoes in a block group, (8192) with block sizes of 1024

//the amount of inodes per block group on a floppy
#define EXT2_I_PER_GRP_FLPPY 184          //derived from 23 blocks for inode table (23 * EXT2_BLOCK_SZ) / (sizeof(ext2_inode_t) [128 bytes])

//the amount of inodes per block group on a filesystem with atlast one full size block group (EXT2_MAX_BLK_GRP_SZ)
#define EXT2_I_PER_GRP       214          //derived from 23 blocks for inode table (23 * EXT2_BLOCK_SZ) / (sizeof(ext2_inode_t) [128 bytes])

///OS creators
#define EXT2_OS_LINUX        0    //Linux
#define EXT2_OS_HURD         1    //GNU HURD
#define EXT2_OS_MASIX        2    //MASIX
#define EXT2_OS_FREEBSD      3    //FreeBSD
#define EXT2_OS_LITES        4    //Lites
#define EXT2_OS_JSOS         5    //JS-OS

///Compresion algorithms
#define EXT2_NO_ALG          0x0  //Binary value of 0x00000000
#define EXT2_LZV1_ALG        0x1  //Binary value of 0x00000001
#define EXT2_LZRW3A_ALG      0x2  //Binary value of 0x00000002
#define EXT2_GZIP_ALG        0x4  //Binary value of 0x00000004
#define EXT2_BZIP2_ALG       0x8  //Binary value of 0x00000008
#define EXT2_LZO_ALG         0x10 //Binary value of 0x00000010

///Superblock related preprocessors
#define EXT2_SBLOCK_OFF      1024 //Superblock is 1024 bytes offset, for partition table
#define EXT2_MAX_MNT_COUNT   20
#define EXT2_CHECK_INTERVAL  3600 //check the file system every hour
#define EXT2_GOOD_OLD_REV    0    //Revision 0
#define EXT2_DYNAMIC_REV     1    //Revision 1 with variable inode sizes, extended attributes, etc.

///File system errors
#define EXT2_VALID_FS        0    //the state of the file system, is ok
#define EXT2_ERROR_FS        1    //the state of the file system, an error has occured (commonly not unmounted properly)

///types of file system errors
#define EXT2_ERRORS_CONTINUE 1    //continue as if nothing has happened
#define EXT2_ERRORS_RO       2    //remount file system as read-only
#define EXT2_ERRORS_PANIC    3    //major error, cause a kernel panic

///Constants relative to the data blocks
//direct blocks
#define EXT2_NDIR_BLOCKS     12                      //direct blocks

//singly indirect blocks
#define EXT2_IND_BLOCK       EXT2_NDIR_BLOCKS        //singly indirect block (12)
#define EXT2_NIND_BLOCK      256                     //singly indirect blocks (calculated by EXT2_BLOCK_SZ / sizeof(u32int))

//doubly indirect blocks
#define EXT2_DIND_BLOCK      (EXT2_IND_BLOCK + 1)    //doubly indirect block (13)
#define EXT2_NDIND_BLOCK     (256 * 256)             //doubly indirect blocks (calculated by squaring EXT2_NIND_BLOCK)

//triply indirect blocks
#define EXT2_TIND_BLOCK      (EXT2_DIND_BLOCK + 1)   //triply indirect block (14)
#define EXT2_NTIND_BLOCK     (256 * 256 * 256)       //triply indirect blocks (calculated by tripling EXT2_NIND_BLOCK)

//number of blocks in the inode typedef
#define EXT2_N_BLOCKS        (EXT2_TIND_BLOCK + 1)   //total number of block (15)

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
  EXT2_I_RUSR = 0x0100,   //user read
  EXT2_I_WUSR = 0x0080,   //user write
  EXT2_I_XUSR = 0x0040,   //user execute
  EXT2_I_RGRP = 0x0020,   //group read
  EXT2_I_WGRP = 0x0010,   //group write
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
  EXT2_SYMLINK,
  EXT2_HARDLINK,
  EXT2_MOUNTPOINT
};

typedef struct ext2_superblock
{
  u32int total_inodes;
  u32int total_blocks;
  u32int reserved_blocks;
  u32int first_data_block;
  u32int block_size;
  u32int blocks_per_group;
  u32int inodes_per_group;
  u32int mtime; //mount time
  u32int wtime; //write time
  u32int utime; //unmount time
  u16int mnt_count;
  u16int max_mnt_count; //max amount of mounts before a check
  u16int magic;
  u16int fs_state;
  u16int error_handling;
  u16int minor_revision_level;
  u32int lastcheck; //the last time the file system was checked
  u32int checkinterval; //maximum amount of time the file system is allowed before being checked
  u32int creator_os;
  u32int revision_level;
  u32int first_inode;
  u16int inode_struct_max;
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
  u32int inode_table_size;  //the amount of EXT2_BLOCK_SZ the inode table is
  u32int gdesc_location;
  u32int reserved;
}ext2_group_descriptor_t;

typedef struct ext2_inode
{
  u32int inode;
  u16int mode;
  u32int type;
  u16int uid;
  u32int size;
  u32int atime;
  u32int ctime;
  u32int mtime;
  u32int dtime;
  u16int gid;
  u16int nlinks;  //number of hard links
  u16int nblocks; //blocks of 512 bytes
  u16int flags;
  u16int osd1;
  u32int blocks[EXT2_N_BLOCKS];
  u16int version;
  u16int fire_acl;
  u16int dir_acl;
  u16int fragment_addr;
  u16int osd2[3];
  u8int reserved[14];
}ext2_inode_t;

struct ext2_dirent
{
  u32int ino;               //inode number. Required by POSIX.
  u16int rec_len;           //bytes from begining of this dirent to end of this dirent, size of file
  u8int name_len;           //the number of bytes of charachters in the name
  u8int file_type;          //a flag that states what type of file this dirent is (ie: a file, pipe, directory, etc.)
  char *name;           //filename, remember to kmalloc this to give it an address, or else it will page fault
};


//adding a hardlink to a directory is exactly the same as adding a file to a directory, so I just make an alias
#define ext2_add_hardlink_to_dir(parent_dir, file, filename)    (ext2_add_file_to_dir(parent_dir, file, EXT2_HARDLINK, filename))

/*set the block group of a floppy*/
u32int ext2_set_block_group(u32int size);

/*set the default super block data*/
void ext2_sblock_set_data(ext2_superblock_t *data, u32int reserved_blocks, u32int sblock_location,
                          u32int error_handling, u32int sblock_group_num, char *partition_name);
                      
/*set the default group descriptor data*/
void ext2_set_gdesc_table(ext2_group_descriptor_t *data);

/*place the meta data (sblock and gdesc) into the ram to be easily read*/
u32int ext2_read_meta_data(ext2_superblock_t **sblock, ext2_group_descriptor_t **gdesc);

/*create a file*/
ext2_inode_t *ext2_create_file(ext2_inode_t *parent_dir, char *name, u32int size);

/*create a directory*/
ext2_inode_t *ext2_create_dir(ext2_inode_t *parentNode, char *name);

/*formats the block bitmap by flipping the correct bits*/
u32int *ext2_format_block_bitmap(ext2_group_descriptor_t *gdesc, u32int blocks_used);

/*writes the locations of the blocks to the inode entries blocks data*/
u32int ext2_inode_entry_blocks(ext2_inode_t *inode, ext2_group_descriptor_t *gdesc, u32int *block_locations, u32int blocks_used);

/*write an inode data to the inode table*/
u32int ext2_data_to_inode_table(ext2_inode_t *data, ext2_group_descriptor_t *gdesc, ext2_superblock_t *sblock);

/*creates a singly block*/
u32int ext2_singly_create(u32int *block_locations, u32int offset, u32int nblocks, ext2_group_descriptor_t *gdesc);

/*creates a doubly block*/
u32int ext2_doubly_create(u32int *block_locations, u32int offset, u32int nblocks, ext2_group_descriptor_t *gdesc);

/*adds a file to a directory*/
u32int ext2_add_file_to_dir(ext2_inode_t *parent_dir, ext2_inode_t *file, u32int file_type, char *name);

/*returns a number of an open inode*/
u32int ext2_find_open_inode(ext2_group_descriptor_t *gdesc);

/*read this block in the block set of a file's inode*/
u32int ext2_block_of_set(ext2_inode_t *file, u32int block_number, u32int *block_output);

/*returns the inode data at a specific locaiton*/
u32int ext2_inode_from_inode_table(u32int inode_number, ext2_inode_t *output, ext2_group_descriptor_t *gdesc);

/*initialize the ext2 filesystem*/
u32int ext2_initialize(u32int size);

/*find a dirent by index from a directory*/
struct ext2_dirent *ext2_dirent_from_dir(ext2_inode_t *dir, u32int index);

/*set the current directory the system is in*/
u32int ext2_set_current_dir(ext2_inode_t *directory);

#endif
