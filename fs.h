// fs.h -- Defines the interface for and structures relating to the virtual file system.
//         Written for JamesM's kernel development tutorials.

#ifndef FS_H
#define FS_H

#include "common.h"

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
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent * (*readdir_type_t)(struct fs_node*,u32int);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*,char *name);

typedef struct fs_node
{
  char name[128];
  u32int mask;              //the permissions mask.
  u32int uid;               //the owning user.
  u32int gid;               //the owning group.
  u32int flags;             //tncludes the node type. See #defines above.
  u32int inode;             //this is device-specific - provides a way for a filesystem to identify files.
  u32int length;            //size of the file, in bytes.
  u32int impl;              //an implementation-defined number.

  u32int blocks[12];        //Data blocks (1 KB each)
  
  read_type_t read;         //function to file read event
  write_type_t write;       //function to file write event
  open_type_t open;         //function to file open event
  close_type_t close;       //function to file close event
  readdir_type_t readdir;   //function to file readdir event
  finddir_type_t finddir;   //function to file finddir event
  struct fs_node *ptr;      //used by mountpoints and symlinks.
}fs_node_t;

struct dirent
{
  u32int ino;               //inode number. Required by POSIX.
  u16int rec_len;           //bytes from begining of this dirent to end of this dirent, size of file
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
void open_fs(fs_node_t *node, u8int read, u8int write);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, u32int index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

/*creates a directory in the ramdisk filesystem*/
fs_node_t *createDirectory(fs_node_t *parentNode, char *name);

/*creates a file in the ramdisk filesystem*/
fs_node_t *createFile(fs_node_t *parentNode, char *name, u32int size);

/*moves a file to a directory*/
int addFileToDir(fs_node_t *dirNode, fs_node_t *fileNode);

/*sets the current directory to a input directory and gets sets the path char array*/
int setCurrentDir(fs_node_t *directory);

/*returns the value of an open inode for a file/directory/etc. */
int findOpenNode();

/*funtion returns 0 on success and 1 on failure, shift the data siftAmount units
 * If shiftAmount < 0, shifts to the left, and shiftAmount > 0, shifts to the right */
int shiftData(void* position, int shiftAmount, u32int lengthOfDataToShift);

/*a given fileSize and block number, it will return how many bytes the file is using in
 * that block number input */
int blockSizeAtIndex(u32int fileSize, u32int blockNum, u32int offset);

#endif
