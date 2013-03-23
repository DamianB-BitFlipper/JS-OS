/*
* fs.c
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


#include "fs.h"
#include "initrd.h"

/*In initrd.c*/
fs_node_t *fs_root = 0; // The root of the filesystem.

extern fs_node_t *initrd_root;             // Parent of root
extern fs_node_t *initrd_dev;              // Root dir
extern fs_node_t *root_nodes;              // List of file nodes.
extern int nroot_nodes;                    // Number of file nodes.
extern u32int fs_location;
extern u32int currentDir_inode;
extern u32int greatestFS_location;

extern char *path;
/*In initrd.c*/

u32int read_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  // Has the node got a read callback?
  if (node->read != 0)
    return node->read(node, offset, size, buffer);
  else
    return 0;
}

u32int write_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  // Has the node got a write callback?
  if (node->write != 0)
    return node->write(node, offset, size, buffer);
  else
    return 0;
}

void open_fs(fs_node_t *node, u8int read, u8int write)
{
  // Has the node got an open callback?
  if (node->open != 0)
    return node->open(node);
}

void close_fs(fs_node_t *node)
{
  // Has the node got a close callback?
  if (node->close != 0)
    return node->close(node);
}

struct dirent *readdir_fs(fs_node_t *node, u32int index)
{
  // Is the node a directory, and does it have a callback?
  if ( (node->flags&0x7) == FS_DIRECTORY && node->readdir != 0 )
    return node->readdir(node, index);
  else
    return 0;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
  // Is the node a directory, and does it have a callback?
  if ( (node->flags&0x7) == FS_DIRECTORY && node->finddir != 0 )
    return node->finddir(node, name);
  else
    return 0;
}

int shiftData(void* position, int shiftAmount, u32int lengthOfDataToShift)
{
  //TODO add right shift functionalbility
  if(shiftAmount < 0) //user wants to shift to the left
  {
    u32int start = (u32int)position, end = (u32int)position + shiftAmount;

    int i; //sifts the data to the left
    for(i = 0; i < lengthOfDataToShift; i++)
    {
      *(char*)(end + i) = *(char*)(start + i);
    }

    memset((u32int*)(end + i), 0, -1 * shiftAmount);
    
    //success!
    return 0;
  }
  
}

int findOpenNode()
{
  //int i;

  //for(i = 0; i < nroot_nodes + 1; i++)
  //{
    ////if all of those headers are not used, then there is no file
    //if(file_headers[i].name == 0 && file_headers[i].offset == 0 && root_nodes[i].name == 0 && root_nodes[i].flags == 0)
    //{
      //break;
      //return i;
    //}
  //}

  nroot_nodes++;
  return nroot_nodes - 1;
}

int blockSizeAtIndex(u32int fileSize, u32int blockNum, u32int offset)
{  
  if(blockNum * BLOCK_SIZE > fileSize)
  {
    //error
    return 0;
  }

  //num is the number of whole (full 1KB) blocks will fit in the file
  int num = (int)(fileSize / BLOCK_SIZE);

  //if the blockNum requested is less than the number of whole blocks, then it will fit BLOCK_SIZE
  if(blockNum < num)
  {
    if(offset == 0) //if there is no offset
    {
      return BLOCK_SIZE;
    }else{
      if((blockNum + 1) * BLOCK_SIZE < offset) //if the offset exceeds the size of the block looked for
      {
        return 0;
      }else{
        if(((blockNum + 1) * BLOCK_SIZE) - offset < BLOCK_SIZE) //if the offset is in the block looked for
        {
          return ((blockNum + 1) * BLOCK_SIZE) - offset; //if the block looked for excedds the block that the offset is in
        }else{
          return BLOCK_SIZE;
        }
      }
    }
    
  }else if(blockNum >= num && blockNum < num + 1)
  {

    if(offset == 0) //if there is no offset
    {
      return fileSize % BLOCK_SIZE;
    }else{
      if((blockNum + 1) * BLOCK_SIZE < offset) //if the offset exceeds the size of the block looked for
      {
        return 0;
      }else{
        if(fileSize - offset < BLOCK_SIZE) //if the offset is in the block looked for
        {
          return fileSize - offset; //if the block looked for excedds the block that the offset is in
        }else{
          return BLOCK_SIZE;
        }
        
      }
    }
        
  }else if((blockNum + 1) * BLOCK_SIZE >= fileSize) //blockNum is too large and does not fir in the fileSize
  {
    return 0;
  }
  
}

fs_node_t *createFile(fs_node_t *parentNode, char *name, u32int size)
{
  int n = findOpenNode();
  // Edit the file's header - currently it holds the file offset
  // relative to the start of the ramdisk. We want it relative to the start
  // of memory.
  //strcpy(file_headers[n].name, name);
  //file_headers[n].offset = file_headers[n - 1].offset + file_headers[n - 1].length;
  //file_headers[n].length = size;

  /* if the offset + length is greater than the current largest address in the filesystem,
   * assign the new value */
  //if(file_headers[n].offset + file_headers[n].length > greatestFS_location)
  //{
    //greatestFS_location = file_headers[n].offset + file_headers[n].length;
  //}
  
  // Create a new file node.
  strcpy(root_nodes[n].name, name);
  root_nodes[n].mask = root_nodes[n].uid = root_nodes[n].gid = 0;
  root_nodes[n].length = size;
  root_nodes[n].inode = n;
  root_nodes[n].flags = FS_FILE;
  root_nodes[n].read = &initrd_read;
  //~ root_nodes[n].write = 0;
  root_nodes[n].write = &initrd_write;
  root_nodes[n].readdir = 0;
  root_nodes[n].finddir = 0;
  root_nodes[n].open = 0;
  root_nodes[n].close = 0;
  root_nodes[n].impl = 0;

  int i;
  for(i = 0; i < (int)((size - 1) / BLOCK_SIZE) + 1; i++) //size - 1 because if size == BLOCK size, there should be only one block created, but w/o that -1, 2 will be created
  {
    root_nodes[n].blocks[i] = kmalloc(BLOCK_SIZE);
  }
  

  addFileToDir(parentNode, &root_nodes[n]);

  return &root_nodes[n];
  
}

int addHardLinkToDir(fs_node_t *hardlink, fs_node_t *directory, char *name)
{
  //this part adds a harlink to this directory with the name .. for the parent directory
  int dirINode = directory->inode;
  int fileINode = hardlink->inode;

  struct dirent dirent;

  int lengthOfName = strlen(name); //"the name is ".." 2 charaters long"

  dirent.ino = fileINode;

  //The size of this whole struct basically
  dirent.rec_len = sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type) + lengthOfName + 1; //+1 is NULL terminating \000
  dirent.name_len = (u8int)lengthOfName;
  dirent.file_type = hardlink->flags;
  dirent.name = (char*)kmalloc(lengthOfName + 1); //+1 being the \000 NULL termination 0 byte at the end of the string

  //with strcp this dirent.name includes the \000 at the end
  memcpy(dirent.name, name, lengthOfName + 1); //".." already had a \000 termination zero
  *(dirent.name + lengthOfName) = 0; //Just in case add a 0

  int i, b;

  for(b = 0; b <= (int)((directory->length - 1) / BLOCK_SIZE); b++) // length - 1 because if length == BLOCK size, there should be only one block checked, but w/o that -1, 2 will be checked
  {
    i = 0;    

    while(*(u16int*)(directory->blocks[b] + i + sizeof(dirent.ino)) != 0)
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i = i + *(u16int*)(directory->blocks[b] + i + sizeof(dirent.ino));

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a block can hold, exit and go to new block
      if(i + dirent.rec_len >= BLOCK_SIZE)
      {
        break;
      }
      
      //if the offset (i) + the length of the contents in the struct dirent is greater than what a direcotory can hold, exit function before page fault happens
      if(i + dirent.rec_len >= directory->length)
      {
        //failed, out of directory left over space
        return 1;
      }
  
    }

    //if i is a valid offset, dont go to a new block, just exit
    if(*(u16int*)(directory->blocks[b] + i + sizeof(dirent.ino)) == 0)
    {
      break;
    }    
    
  }
  
  //assigns the contents of the struct dirent to the directory contents location
  memcpy((u32int*)(directory->blocks[b] + i), &dirent, dirent.rec_len - dirent.name_len);

  strcpy((char*)(directory->blocks[b] + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), dirent.name);

  //~ k_printf("\nhex dirent.name address is: %h and the text is: %s \nand copied is %s and i is: %d\n", dirent.name, dirent.name, (char*)(file_headers[dirINode].offset + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), i);
  
  kfree(dirent.name);

  //success!
  return 0;
  
  //this part adds a harlink to this directory with the name .. for the parent directory
}

fs_node_t *createDirectory(fs_node_t *parentNode, char *name)
{
  if(parentNode != initrd_root) //if the directory we are creating is not root
  {
    int i, length = strlen(name);
    for(i = 0; i < length; i++) //check if the dirName has any "/" as a dir name cannot contain them
    {
      if(*(name + i) == '/')
      {
        k_printf("The directory name: \"%s\" may not contain any \"/\" characters\n", name);

        //failure!
        return 0;
      }
    }
  }
  
  int n = findOpenNode();
  
  //puts root node information for this directory
  strcpy(root_nodes[n].name, name);

  root_nodes[n].mask = root_nodes[n].uid = root_nodes[n].gid = 0;
  root_nodes[n].length = DIRECTORY_SIZE;
  root_nodes[n].inode = n;  
  root_nodes[n].flags = FS_DIRECTORY;

  root_nodes[n].read = 0;
  root_nodes[n].write = 0;
  root_nodes[n].open = 0;
  root_nodes[n].close = 0;
  root_nodes[n].readdir = &initrd_readdir;
  root_nodes[n].finddir = &initrd_finddir;
  root_nodes[n].ptr = 0;
  root_nodes[n].impl = 0;

  int i;
  for(i = 0; i < (int)((root_nodes[n].length - 1) / BLOCK_SIZE) + 1; i++) //size - 1 because if size == BLOCK size, there should be only one block created, but w/o that -1, 2 will be created
  {
    root_nodes[n].blocks[i] = kmalloc(BLOCK_SIZE);

    //set all contents of this directory to 0    
    memset(root_nodes[n].blocks[i], 0, BLOCK_SIZE);    
  }

  if(parentNode != initrd_root)
  {
    addFileToDir(parentNode, &root_nodes[n]);

    addHardLinkToDir(&root_nodes[n], &root_nodes[n], "."); //adds hardlink for the current directory
    addHardLinkToDir(parentNode, &root_nodes[n], ".."); //adds hardlink for the parent directory
    
  }

  return &root_nodes[n];

}

int addFileToDir(fs_node_t *dirNode, fs_node_t *fileNode)
{
  int dirINode = dirNode->inode;
  int fileINode = fileNode->inode;

  struct dirent dirent;

  int lengthOfName = strlen(fileNode->name);

  dirent.ino = fileINode;

  //The size of this whole struct basically
  dirent.rec_len = sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type) + lengthOfName + 1; //+1 is NULL terminating \000
  dirent.name_len = (u8int)lengthOfName;
  dirent.file_type = fileNode->flags;
  dirent.name = (char*)kmalloc(lengthOfName + 1); //+1 being the \000 NULL termination 0 byte at the end of the string

  //with strcp this dirent.name includes the \000 at the end
  //~ strcpy(dirent.name, fileNode->name);
  memcpy(dirent.name, fileNode->name, lengthOfName + 1); //fileNode->name already had a \000 termination zero
  *(dirent.name + lengthOfName) = 0; //Just in case add a 0

  int i, b;

  for(b = 0; b <= (int)((dirNode->length - 1) / BLOCK_SIZE); b++) // length - 1 because if length == BLOCK size, there should be only one block checked, but w/o that -1, 2 will be checked
  {
    i = 0;
  
    //~ while(*(u16int*)(file_headers[dirINode].offset + i + sizeof(dirent.ino)) != 0)
    while(*(u16int*)(dirNode->blocks[b] + i + sizeof(dirent.ino)) != 0)
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      //~ i = i + *(u16int*)(file_headers[dirINode].offset + i + sizeof(dirent.ino));
      i = i + *(u16int*)(dirNode->blocks[b] + i + sizeof(dirent.ino));
      
      //if the offset (i) + the length of the contents in the struct dirent is greater than what a block can hold, exit and go to new block
      if(i + dirent.rec_len >= BLOCK_SIZE)
      {
        break;
      }
      
      //if the offset (i) + the length of the contents in the struct dirent is greater than what a direcotory can hold, exit function before page fault happens
      if(i + dirent.rec_len >= dirNode->length)
      {
        //failed, out of directory left over space
        return 1;
      }

    }

    //if i is a valid offset, dont go to a new block, just exit
    if(*(u16int*)(dirNode->blocks[b] + i + sizeof(dirent.ino)) == 0)
    {
      break;
    }
    
  }

  //assigns the contents of the struct dirent to the directory contents location
  //~ memcpy((u32int*)(file_headers[dirINode].offset + i), &dirent, dirent.rec_len - dirent.name_len);
  memcpy((u32int*)(dirNode->blocks[b] + i), &dirent, dirent.rec_len - dirent.name_len);

  //~ strcpy((char*)(file_headers[dirINode].offset + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), dirent.name);
  strcpy((char*)(dirNode->blocks[b] + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), dirent.name);

  //~ k_printf("\nhex dirent.name address is: %h and the text is: %s \nand copied is %s and i is: %d\n", dirent.name, dirent.name, (char*)(file_headers[dirINode].offset + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), i);
  
  kfree(dirent.name);

  //success!
  return 0;
}

int setCurrentDir(fs_node_t *directory)
{
  kfree(path); //frees the contents of the char array pointer, path
  
  currentDir_inode = directory->inode; //sets the value of the dir inode to the cuurentDir_inode

  fs_node_t *node = directory;
  fs_node_t *copy;
  
  int count = 0, totalCharLen = 0;

  do
  {
    copy = node;    
    
    if(copy != 0)
    {
      totalCharLen = totalCharLen + strlen(copy->name) + 1; //+1 being the preceding "/" to every dir that will be added later
      count++;
    }
    
    node = finddir_fs(copy, "..");
  }
  while(node != copy);

  if(count > 1) //if we are in a directory other than root
  {
    /* We have the root dir
     * that does not need a preceding "/" because we will get "//"
     * which is ugly and not right. Also the "/" before the very first
     * directory should not be there because it will look ugly with
     * the root "/". So the total totalCharLen should be
     * -2 to count for both of those instances*/
    totalCharLen = totalCharLen - 2;
     
    path = (char*)kmalloc(totalCharLen + 1); //+1 is for the \000 NULL terminating 0
    //~ strcpy(path, "/");

    copy = directory;

    int i, charsWritten = 0, nameLen;
    for(i = 0; i < count; i++)
    {
      nameLen = strlen(copy->name);

      /* i < count - 2 is a protection from drawing the preceding "/"
       * on the first two dirs in the path (root and one more). The first two dirs will
       * allways be drawn the last two times, thus if i is less
       * than the count - 2, that means we are not yet at the last
       * two drawing and it is ok to have a precedding "/" */
      if(copy != fs_root && i < count - 2)
      {
        memcpy(path + (totalCharLen - charsWritten - nameLen - 1), "/", 1);
        memcpy(path + (totalCharLen - charsWritten - nameLen), copy->name, nameLen);
        charsWritten = charsWritten + nameLen + 1; //increment charsWritten with the "/<name>" string we just wrote, +1 is that "/"
        
      }else{
        memcpy(path + (totalCharLen - charsWritten - nameLen), copy->name, nameLen);
        charsWritten = charsWritten + nameLen; //increment charsWritten with the "/" string we just wrote
        
      }
      
      node = finddir_fs(copy, "..");
      copy = node;
    }
    
  }else{
    path = (char*)kmalloc(2); //2 chars beign "/" for root and \000
    strcpy(path, "/");
    *(path + 1) = 0; //added \000 to the end
  }

  //~ k_printf("\n\nPATH is: %s\n", path);
  return 0;
}
