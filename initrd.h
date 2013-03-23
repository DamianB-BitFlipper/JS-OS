// initrd.h -- Defines the interface for and structures relating to the initial ramdisk.
//             Written for JamesM's kernel development tutorials.

#ifndef INITRD_H
#define INITRD_H

#include "common.h"
#include "fs.h"

#define MAX_NUMBER_OF_DIRECTORIES     128
#define DIRECTORY_SIZE                1024 //the size of the directory (bytes)
#define MAX_NUMBER_OF_FILES           128
#define FS_SIZE                       sizeof(fs_node_t) * 16384
#define BLOCK_SIZE                    1024

extern fs_node_t *root_nodes;              // List of file nodes.
extern u32int currentDir_inode; //the inode of the current directory

typedef struct
{
  u32int nfiles; // The number of files in the ramdisk.
}initrd_header_t;

typedef struct
{
  u8int magic;     // Magic number, for error checking.
  s8int name[128];  // Filename.
  u32int offset;   // Offset in the initrd that the file starts.
  u32int length;   // Length of the file.
}initrd_file_header_t;

// Initialises the initial ramdisk. It gets passed the address of the multiboot module,
// and returns a completed filesystem node.
fs_node_t *initialise_initrd(u32int location);

struct dirent *initrd_readdir(fs_node_t *dirNode, u32int index);
fs_node_t *initrd_finddir(fs_node_t *dirNode, char *name);

/*reads from file*/
u32int initrd_read(fs_node_t *node, u32int offset, u32int size, u8int *buffer);

/*writes to a file*/
u32int initrd_write(fs_node_t *node, u32int offset, u32int size, u8int *buffer);

#endif
