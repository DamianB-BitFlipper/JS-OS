/*
* vfs.c
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

#include <system.h>

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
  if(node->read)
    return node->read(node, offset, size, buffer);
  else
    return 0;
}

u32int write_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  // Has the node got a write callback?
  if(node->write)
    return node->write(node, offset, size, buffer);
  else
    return 0;
}

FILE *open_fs(char *filename, fs_node_t *dir)
{
  // Has the node got an open callback?
  if((dir->flags & 0x7) == FS_DIRECTORY && dir->finddir != 0)
  {
    fs_node_t *file;
    file = finddir_fs(dir, filename);

    if(file->read)
    {
      FILE *buffer;
      buffer = (FILE*)kmalloc(file->length);

      read_fs(file, 0, file->length, buffer);

      return buffer;
    }else
      return 0;
  }else
    return 0;
}

u32int *close_fs(FILE *file)
{
  if(file)
  {
    kfree((void*)file);
    return 0;
  }
  else
    return (u32int*)1;
}

struct dirent *readdir_fs(fs_node_t *node, u32int index)
{
  // Is the node a directory, and does it have a callback?
  if((node->flags & 0x7) == FS_DIRECTORY && node->readdir != 0)
    return node->readdir(node, index);
  else
    return 0;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
  // Is the node a directory, and does it have a callback?
  if((node->flags & 0x7) == FS_DIRECTORY && node->finddir != 0)
    return node->finddir(node, name);
  else
    return 0;
}

int shiftData(void *position, int shiftAmount, u32int lengthOfDataToShift)
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
  int i;

  for(i = 0; i < nroot_nodes; i++)
  {
    //if all of those headers are not used, then there is no file
    if(*(root_nodes[i].name) == 0 && root_nodes[i].inode == 0 && root_nodes[i].mask == 0 && root_nodes[i].flags == 0)
    {
      return i;
    }
  }

  nroot_nodes++;
  return nroot_nodes - 1;
}

u32int blockSizeAtIndex(u32int fileSize, u32int blockNum, u32int offset)
{
  if(blockNum * BLOCK_SIZE > fileSize)
  {
    //error
    return 0;
  }

  //num is the number of whole (full 1KB) blocks will fit in the file
  u32int num = (u32int)(fileSize / BLOCK_SIZE);

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

u32int *block_of_set(fs_node_t *node, u32int block_number)
{
  //if the block is a direct block
  if(block_number >= 0 && block_number < BLOCKS_DIRECT)
  {
    return &node->blocks[block_number];
    
  //if the block is in the singly set
  }else if(block_number >= BLOCKS_DIRECT && block_number < BLOCKS_DIRECT + BLOCKS_SINGLY)
  {
    return &node->singly->blocks[block_number];
    
  //if the block is in the doubly set
  }else if(block_number >= BLOCKS_DIRECT + BLOCKS_SINGLY &&
          block_number < BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY)
  {
    return &node->doubly->singly->blocks[block_number];

  //if the block is in the triply set
  }else if(block_number >= BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY&&
          block_number < BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY + BLOCKS_TRIPLY)
  {
    return &node->triply->doubly->singly->blocks[block_number];

  }else{
    //the block number is not in range, error
    return 0;
  }
}

fs_node_t *createFile(fs_node_t *parentNode, char *name, u32int size)
{
  u32int n = findOpenNode();

  // Create a new file node.
  strcpy(root_nodes[n].name, name);
  root_nodes[n].mask = 0b110110100; //user rw, group rw, other r
  root_nodes[n].uid = root_nodes[n].gid = 0;
  root_nodes[n].length = size;
  root_nodes[n].inode = n;
  root_nodes[n].flags = FS_FILE;
  root_nodes[n].read = &initrd_read;
  //~ root_nodes[n].write = 0;
  root_nodes[n].write = &initrd_write;
  root_nodes[n].readdir = 0;
  root_nodes[n].finddir = 0;
  root_nodes[n].impl = 0;


  u32int i, allocSize, block;
  for(i = 0; i < (u32int)((size - 1) / BLOCK_SIZE) + 1; i++) //size - 1 because if size == BLOCK size, there should be only one block created, but w/o that -1, 2 will be created
  {
   
    //get the address of the block to write to
    block = (u32int)block_of_set(&root_nodes[n], i);
    
    /*The following examples assume that we are in the direct blocks of the node
     * the concept is the same for the indirect blocks
     *
     * (u32int*)block points to the address of the rootnodes[i].blocks[a]
     * e.g. &rootnodes[i].blocks[a]
     * 
     * then the *(u32int*)block is the value of that address, i.e. it impersonates
     * that it (u32int block) is rootnodes[i].blocks[a], although
     * they are found at different memory locations, same concept with hard links */
    
    //allocate space for a block
    *(u32int*)block = kmalloc(BLOCK_SIZE);

  }


  addFileToDir(parentNode, &root_nodes[n]);

  return &root_nodes[n];

}

int addHardLinkToDir(fs_node_t *hardlink, fs_node_t *directory, char *name)
{
  //this part adds a harlink to this directory with the name .. for the parent directory
  u32int dirINode = directory->inode;
  u32int fileINode = hardlink->inode;

  struct dirent dirent;

  u32int lengthOfName = strlen(name);

  dirent.ino = fileINode;

  //The size of this whole struct basically
  dirent.rec_len = sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type) + lengthOfName + 1; //+1 is NULL terminating \000
  dirent.name_len = (u8int)lengthOfName;
  dirent.file_type = hardlink->flags;

  //+1 being the \000 NULL termination 0 byte at the end of the string  
  dirent.name = (char*)kmalloc(lengthOfName + 1);

  //with strcp this dirent.name includes the \000 at the end
  memcpy(dirent.name, name, lengthOfName + 1); //name already had a \000 termination zero
  *(dirent.name + lengthOfName) = 0; //Just in case add a 0

  u32int i, b, block;

  for(b = 0; b <= (u32int)((directory->length - 1) / BLOCK_SIZE); b++) // length - 1 because if length == BLOCK size, there should be only one block checked, but w/o that -1, 2 will be checked
  {
    i = 0;

    block = (u32int)block_of_set(directory, b);
    

    while(*(u16int*)(*(u32int*)block + i + sizeof(dirent.ino)) != 0)
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i = i + *(u16int*)(*(u32int*)block + i + sizeof(dirent.ino));

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
    if(*(u16int*)(*(u32int*)block + i + sizeof(dirent.ino)) == 0)
    {
      break;
    }

  }

  //assigns the contents of the struct dirent to the directory contents location
  memcpy((u32int*)(*(u32int*)block + i), &dirent, dirent.rec_len - dirent.name_len);

  strcpy((char*)(*(u32int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), dirent.name);

  kfree(dirent.name);

  //success!
  return 0;

}

fs_node_t *createDirectory(fs_node_t *parentNode, char *name)
{
  /*if the directory we are creating is not root, initrd_root is the
   * "mother root" directory, a directory that contains root and only root
   * directory, we can pass the '/' check for the root directory */
  if(parentNode != initrd_root)
  {
    
    //check if the dirName has any "/" as a dir name cannot contain them
    u32int i, length = strlen(name);
    for(i = 0; i < length; i++)
    {
      if(*(name + i) == '/')
      {
        k_printf("The directory name: \"%s\" may not contain any \"/\" characters\n", name);
  
        //error!
        return 0;
      }
    }
    
  }

  u32int n = findOpenNode();

  //puts root node information for this directory
  strcpy(root_nodes[n].name, name);

  root_nodes[n].mask = 0b111111101; //user rwx, group rwx, other rx
  root_nodes[n].uid = root_nodes[n].gid = 0;
  root_nodes[n].length = DIRECTORY_SIZE;
  root_nodes[n].inode = n;
  root_nodes[n].flags = FS_DIRECTORY;

  root_nodes[n].read = 0;
  root_nodes[n].write = 0;
  root_nodes[n].readdir = &initrd_readdir;
  root_nodes[n].finddir = &initrd_finddir;
  root_nodes[n].ptr = 0;
  root_nodes[n].impl = 0;

  /*size - 1 because if size == BLOCK size, there should be only one
   * block created, but w/o that -1, 2 will be created,
   * also, we reuse the u32int variable i again to save memory */
  u32int i, block;
  for(i = 0; i < (u32int)((root_nodes[n].length - 1) / BLOCK_SIZE) + 1; i++)
  {
    block = (u32int)block_of_set(&root_nodes[n], i);
    
    *(u32int*)block = kmalloc(BLOCK_SIZE);

    //set all contents of this directory to 0
    memset(*(u32int*)block, 0, BLOCK_SIZE);
  }

  /*initrd_root is the "mother root" directory, a directory
   * that contains root and only root directory, if the parentNode is
   * initrd_root, then we are creating root, and root has its own special
   * hard links that are made outside of this function, else, make those hard
   * links */
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
  u32int dirINode = dirNode->inode;
  u32int fileINode = fileNode->inode;

  struct dirent dirent;

  u32int lengthOfName = strlen(fileNode->name);

  dirent.ino = fileINode;

  //The size of this whole struct basically
  dirent.rec_len = sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type) + lengthOfName + 1; //+1 is NULL terminating \000
  dirent.name_len = (u8int)lengthOfName;
  dirent.file_type = fileNode->flags;
  
  //+1 being the \000 NULL termination 0 byte at the end of the string
  dirent.name = (char*)kmalloc(lengthOfName + 1);
  
  //copy the name of the file (fileNode->name) to the dirent.name
  memcpy(dirent.name, fileNode->name, lengthOfName + 1); //fileNode->name already had a \000 termination zero
  *(dirent.name + lengthOfName) = 0; //Just in case add a \000 at the end

  u32int i, b, block;

  /*In this section, we find a valid offset (i) and block number (b)
   * to an open dirent space
   *
   * length - 1 because if length == BLOCK size, there should be only one
   * block checked, but w/o that -1, 2 will be checked */
  for(b = 0; b <= (u32int)((dirNode->length - 1) / BLOCK_SIZE); b++)
  {
    i = 0;
    block = (u32int)block_of_set(dirNode, b);

    //loop untill we hit the end of the current block
    while(*(u16int*)(*(u32int*)block + i + sizeof(dirent.ino)) != 0)
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i = i + *(u16int*)(*(u32int*)block + i + sizeof(dirent.ino));

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

    //if i is a valid offset, do not go to a new block, just exit
    if(*(u16int*)(*(u32int*)block + i + sizeof(dirent.ino)) == 0)
    {
      break;
    }

  }

  //assigns the contents of the struct dirent to the directory contents location
  memcpy((u32int*)(*(u32int*)block + i), &dirent, dirent.rec_len - dirent.name_len);

  strcpy((char*)(*(u32int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type)), dirent.name);


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

  u32int count = 0, totalCharLen = 0;

  /*starts from the current directory, goes backwards by getting the node
   * of the parent (looking up the data in ".." dir), adding its namelen
   * to the u32int totalCharLen and getting its parent, etc.
   *
   *once the parent is the same as the child, that only occurs with the
   * root directory, so we should exit */
  do
  {
    copy = node;

    if(copy != 0)
    {
      //+1 being the preceding "/" to every dir that will be added later
      totalCharLen = totalCharLen + strlen(copy->name) + 1; 
      count++;
    }

    node = finddir_fs(copy, "..");
  }
  while(node != copy);

  if(count > 1) //if we are in a directory other than root
  {
    /*we have the root dir
     * that does not need a preceding "/" because we will get "//"
     * which is ugly and not right. Also the "/" before the very first
     * directory should not be there because it will look ugly with
     * the root "/". So the total totalCharLen should be
     * -2 to count for both of those instances*/
    totalCharLen = totalCharLen - 2;

    path = (char*)kmalloc(totalCharLen + 1); //+1 is for the \000 NULL terminating 0
    //~ strcpy(path, "/");

    //reset the copy back to the top (current directory)
    copy = directory;

    u32int i, charsWritten = 0, nameLen;
    for(i = 0; i < count; i++)
    {
      nameLen = strlen(copy->name);

      /* i < count - 2 is a protection from drawing the preceding "/"
       * on the first two dirs in the path (root and one more). The first two dirs will
       * allways be drawn the last two times (we write the dir names to path from
       * current dir (top) to root (bottom)), thus if i is less
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

      //find the parent of copy
      node = finddir_fs(copy, "..");
      copy = node;
    }

    *(path + totalCharLen) = 0; //added \000 to the end of path

  }else{
    //keep it simple, if root is the only directory, copy its name manually
    path = (char*)kmalloc(2); //2 chars beign "/" for root and \000

    *(path) = '/';
    *(path + 1) = 0; //added \000 to the end
  }

  //~ k_printf("\n\nPATH is: %s\n", path);
  
  //sucess!
  return 0;
}
