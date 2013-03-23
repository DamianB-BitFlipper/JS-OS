// initrd.c -- Defines the interface for and structures relating to the initial ramdisk.
//             Written for JamesM's kernel development tutorials.

#include "common.h"
#include "initrd.h"
#include "fs.h"

initrd_header_t *initrd_header;     // The header.
initrd_file_header_t *file_headers; // The list of file headers.
fs_node_t *initrd_root;             // Our root directory node.
fs_node_t *initrd_dev;              // We also add a directory node for /dev, so we can mount devfs later on.
fs_node_t *root_nodes;              // List of file nodes.
int nroot_nodes;                    // Number of file nodes.

//~ fs_node_t *directory_nodes;         // List of file nodes.
u32int fs_location;
u32int currentDir_inode; //the inode of the current directory

char *path; //a character array containing the path from root to the current directory

extern u32int initrd_location;
extern u32int initrd_end;

struct dirent dirent;

//~ static u32int initrd_read(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
u32int initrd_read(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  int startingBlock = (int)(offset / BLOCK_SIZE);
  int endingBlock = (int)((offset + size) / BLOCK_SIZE);

  //~ initrd_file_header_t header = file_headers[node->inode];

  if (offset > node->length)
  {
    return 0;
  }

  if (offset+size > node->length)
  {
    size = node->length - offset;
  }


  //this memcpy is outside of the for loop because it has the offset
  //memcpy(buffer + startingBlock * BLOCK_SIZE, (u8int*)(node->blocks[startingBlock] + offset), size);

  //TODO add offset support
  int i; //i starts off at startingBlock + 1 because we already copies the block above
  for(i = startingBlock; i < endingBlock + 1; i++)
  {
    memcpy(buffer + i * BLOCK_SIZE, (u8int*)(node->blocks[i]), blockSizeAtIndex(node->length, i, offset));
  }

  //~ memcpy(buffer, (u8int*)(header.offset + offset), size);
  return size;
}

u32int initrd_write(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  int startingBlock = (int)(offset / BLOCK_SIZE);
  int endingBlock = (int)((offset + size) / BLOCK_SIZE);

  //~ initrd_file_header_t header = file_headers[node->inode];

  if (offset > node->length)
  {
    return 0;
  }

  if (offset + size > node->length)
  {
    size = node->length - offset;
  }

  //this memcpy is outside of the for loop because it has the offset
  //memcpy((u8int*)(node->blocks[startingBlock] + offset), buffer + startingBlock * BLOCK_SIZE, size);

  //TODO add offset support
  int i; //i starts off at startingBlock + 1 because we already copies the block above
  for(i = startingBlock; i < endingBlock + 1; i++)
  {
    memcpy((u8int*)(node->blocks[i]), buffer + i * BLOCK_SIZE, blockSizeAtIndex(node->length, i, offset));
  }

  //~ memcpy((u8int*)(node->blocks[startingBlock] + offset), buffer, size);
  //~ memcpy((u8int*)(header.offset + offset), buffer, size);
  return size;
}

//~ static struct dirent *initrd_readdir(fs_node_t *node, u32int index)
struct dirent *initrd_readdir(fs_node_t *dirNode, u32int index)
{

  if(dirNode->flags = FS_DIRECTORY) //just to check if the input node is a directory
  {

    int i = 0;
    int loop = 0, block = 0;

    //~ while(*(u32int*)(file_headers[node->inode].offset + i) != 0)
    while(1)
    {

      if(loop == index)
      {
        //~ if(*(u32int*)(file_headers[node->inode].offset + i) != 0) //there is a file struct dirent at that offset (i)
        //~ if(*(u16int*)(file_headers[dirNode->inode].offset + i + sizeof(dirent.ino)) != 0)
        if(*(u16int*)(dirNode->blocks[block] + i + sizeof(dirent.ino)) != 0)
        {
          static struct dirent dirent2;

          dirent2.ino = *(u32int*)(dirNode->blocks[block] + i);
          dirent2.rec_len = *(u16int*)(dirNode->blocks[block] + i + sizeof(dirent2.ino));
          dirent2.name_len = *(u8int*)(dirNode->blocks[block] + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len));
          dirent2.file_type = *(u8int*)(dirNode->blocks[block] + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len));

          /* Here I use memcpy instead of strcpy because the root_nodes[dirent2.ino].name
           * is 256 bytes big, yet the dirent2.name should contain ONLY the charaters and the
           * terminating \000 at the end, with strcpy all 256 bytes will be transfered, yet I
           * would only want the first dirent2.name_len + 1 of that string the the root_nodes */
          //memcpy(dirent2.name, root_nodes[dirent2.ino].name, dirent2.name_len);
          //*(dirent2.name + dirent2.name_len) = 0; //Adds terminating 0 to dirent2.name


          ///*SO I MADE THIS WORK, YET IT WOULD BE MUCH MORE EFFICIENT TO GET THE NAME BY LOOKING IT UP IN THE root_node[dirent2.ino].name *///
          dirent2.name = (char*)kmalloc(dirent2.name_len + 1); //+1 being the terminating NULL 0 at the end on the name string

          memset(dirent2.name, 0, dirent2.name_len + 1); //clears junk that may be contained in memory when kmallocing

          //~ strcpy(dirent2.name, (char*)(file_headers[node->inode].offset + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len) + sizeof(dirent2.file_type)) );
          memcpy(dirent2.name, (char*)(dirNode->blocks[block] + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len) + sizeof(dirent2.file_type)), dirent2.name_len + 1 );

          *(dirent2.name + dirent2.name_len) = 0; //Adds terminating 0 to string
          ///*SO I MADE THIS WORK, YET IT WOULD BE MUCH MORE EFFICIENT TO GET THE NAME BY LOOKING IT UP IN THE root_node[dirent2.ino]->name*///

          //~ k_printf("\nBUFFER, %s %d\n", dirent2.name, dirent2.ino);

          //~ kfree(dirent2.name);

          return &dirent2;

        }else{
          return 0;
        }

      //~ }else if(i + *(u16int*)(file_headers[node->inode].offset + i + 4) + dirent.rec_len >= DIRECTORY_SIZE) //if the offset (i) + the length of the contents in the struct dirent is greater than what a direcotory can hold, exit function before page fault happens
      //~ {

      }else{

        //there are more dirents after this one
        //~ if(*(u16int*)(file_headers[dirNode->inode].offset + i + sizeof(dirent.ino)) != 0)
        if(*(u16int*)(dirNode->blocks[block] + i + sizeof(dirent.ino)) != 0)
        {
          //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
          //~ i = i + *(u16int*)(file_headers[dirNode->inode].offset + i + sizeof(dirent.ino));
          i = i + *(u16int*)(dirNode->blocks[block] + i + sizeof(dirent.ino));
          loop++;
        }else{ //this is the last direct, add 1 to block and reset the offset (i)
          i = 0;
          block++;
        }
        
      }

    }

  }else{
    return 0;
  }
}

//~ static fs_node_t *initrd_finddir(fs_node_t *node, char *name)
fs_node_t *initrd_finddir(fs_node_t *dirNode, char *name)
{
  if (dirNode == initrd_root && !strcmp(name, "/") )
    return initrd_dev;

  if(dirNode->flags == FS_DIRECTORY)
  {
    struct dirent *dirent2;

    int i;
    for (i = 0; i < nroot_nodes; i++)
    {
      dirent2 = initrd_readdir(dirNode, i);

      if(dirent2 != 0)
      {
        if(strcmp(name, dirent2->name) == 0) //the name input matches to the dirent2.name that I got out
        {
          return &root_nodes[dirent2->ino];
        }
      }
    }

  }

  //no file found
  return 0;
}

fs_node_t *initialise_initrd(u32int location)
{
  fs_location = kmalloc(FS_SIZE);

  u32int tempLoc = kmalloc(initrd_end - initrd_location); //random size, make it defined

  //size = the size of the initrd module (address of end - address of start), end and start are defined by the bootloader
  memcpy(tempLoc, location, initrd_end - initrd_location);
  //~ memcpy(fs_location, location, initrd_end - initrd_location);

  // Initialise the main and file header pointers and populate the root directory.
  initrd_header = (initrd_header_t *)tempLoc; //first part of location is the number of headers (in struct)
  //~ initrd_header = (initrd_header_t *)fs_location; //first part of location is the number of headers (in struct)

  //the location offset with the struct that contains the number of headers
  file_headers = (initrd_file_header_t *)(tempLoc + sizeof(initrd_header_t));
  //~ file_headers = (initrd_file_header_t *)(fs_location + sizeof(initrd_header_t));

  // Initialise the root directory.
  initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
  strcpy(initrd_root->name, "/");
  initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
  initrd_root->flags = FS_DIRECTORY;
  initrd_root->read = 0;
  initrd_root->write = 0;
  initrd_root->open = 0;
  initrd_root->close = 0;
  initrd_root->readdir = &initrd_readdir;
  initrd_root->finddir = &initrd_finddir;
  initrd_root->ptr = 0;
  initrd_root->impl = 0;

  // Initialise the /dev directory (required!)
  //~ initrd_dev = (fs_node_t*)kmalloc(sizeof(fs_node_t));
  //~ strcpy(initrd_dev->name, "/");
  //~ initrd_dev->mask = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
  //~ initrd_dev->flags = FS_DIRECTORY;
  //~ initrd_dev->read = 0;
  //~ initrd_dev->write = 0;
  //~ initrd_dev->open = 0;
  //~ initrd_dev->close = 0;
  //~ initrd_dev->readdir = &initrd_readdir;
  //~ initrd_dev->finddir = &initrd_finddir;
  //~ initrd_dev->ptr = 0;
  //~ initrd_dev->impl = 0;

  //~ root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * (initrd_header->nfiles + MAX_NUMBER_OF_FILES));
  root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * (MAX_NUMBER_OF_DIRECTORIES * MAX_NUMBER_OF_FILES));
  nroot_nodes = initrd_header->nfiles;

  ///CHECK IF THIS IS USED///

  //~ directory_nodes = (fs_node_t*)kmalloc(MAX_NUMBER_OF_DIRECTORIES * (sizeof(fs_node_t) * MAX_NUMBER_OF_FILES));
  ///CHECK IF THIS IS USED///

  //~ initrd_dev = createDirectory(initrd_root, "/");
//~
  //~ setCurrentDir(initrd_dev);

  //~ path = (char*)kmalloc(2); //by default, the path will start at root "/" and will contain as contents "/", \000, 2 characters
  //~ strcpy(path, "/"); //default name

  // For every file...
  int i, a, allocSize;
  for (i = 0; i < initrd_header->nfiles; i++)
  {
    // Edit the file's header - currently it holds the file offset
    // relative to the start of the ramdisk. We want it relative to the start
    // of memory.
    file_headers[i].offset += tempLoc;
    //~ file_headers[i].offset += fs_location;
    // Create a new file node.
    strcpy(root_nodes[i].name, &file_headers[i].name);
    root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
    root_nodes[i].length = file_headers[i].length;
    root_nodes[i].inode = i;
    root_nodes[i].flags = FS_FILE;
    root_nodes[i].read = &initrd_read;
    //~ root_nodes[i].write = 0;
    root_nodes[i].write = &initrd_write;
    root_nodes[i].readdir = 0;
    root_nodes[i].finddir = 0;
    root_nodes[i].open = 0;
    root_nodes[i].close = 0;
    root_nodes[i].impl = 0;

    for(a = 0; a < (int)(((root_nodes[i].length + 1) / BLOCK_SIZE) + 1); a++)
    {
      allocSize = blockSizeAtIndex(root_nodes[i].length, a, 0);

      root_nodes[i].blocks[a] = kmalloc(BLOCK_SIZE);
      memcpy(root_nodes[i].blocks[a], file_headers[i].offset + a * BLOCK_SIZE, allocSize);
    }

  }


  initrd_dev = createDirectory(initrd_root, "/");

  addHardLinkToDir(initrd_dev, initrd_dev, "."); //adds hardlink for root
  addHardLinkToDir(initrd_dev, initrd_dev, ".."); //adds hardlink for root

  setCurrentDir(initrd_dev);

  //assigns all of the files that were in the file_headers to this dev directory
  for(i = 0; i < initrd_header->nfiles; i++)
  {
    addFileToDir(initrd_dev, &root_nodes[i]);
  }

  kfree(tempLoc);

  //~ return initrd_root;
  return initrd_dev;
}
