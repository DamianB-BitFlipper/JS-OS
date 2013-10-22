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

//the curent directory node
extern void *ptr_currentDir;

extern u32int greatestFS_location;

extern char *path;
/*In initrd.c*/

u32int read_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  // Has the node got a read callback?
  if(node->read)
  {

    file_desc_t *fdesc;
    fdesc = look_up_fdesc(node);

    //we did not find the file desc in the list
    if(!fdesc)
    {
      k_printf("Error: file not in file descriptor list\n");
      return 1;
    }

    //if the user can read from it
    if(fdesc->permisions & FDESC_READ)
      return node->read(node, offset, size, buffer);
    else
      k_printf("Error: reading from an unprivilaged file\n");
  }
  
  //if we have not exited yet, it is an error
  return 1;
}

u32int write_fs(fs_node_t *node, u32int offset, u32int size, u8int *buffer)
{
  // Has the node got a write callback?
  if(node->write)
  {
    file_desc_t *fdesc;
    fdesc = look_up_fdesc(node);

    //we did not find the file desc in the list
    if(!fdesc)
    {
      k_printf("Error: file not in file descriptor list\n");
      return 1;
    }

    //if the user can read from it
    if(fdesc->permisions & FDESC_WRITE)
      return node->write(node, offset, size, buffer);
    else
      k_printf("Error: writing to an unprivilaged file\n");
  }

  //if we have not exited yet, it is an error
  return 1;
}

FILE *open_fs(char *filename, fs_node_t *dir, char *mask)
{
  // Has the node got an open callback?
  if((dir->flags & 0x7) == FS_DIRECTORY && dir->finddir)
  {
    fs_node_t *file;
    file = finddir_fs(dir, filename);

    if(file && file->read)
    {
      file_desc_t *tmp_desc, *new_desc;
      tmp_desc = initial_fdesc;

      //a simple error check if the tmp_desc exists
      if(!tmp_desc)
        return 0;

      /*go to the end of out file descriptor list
       * while iterating, check if this file_desc already exists,
       * if true, return an error*/
      for(; tmp_desc->next; tmp_desc = tmp_desc->next)
        //if we already have this file node in the list
        if(tmp_desc->node == file)
          return tmp_desc; //no need to open, just return it

      new_desc = (file_desc_t*)kmalloc(sizeof(file_desc_t));
      
      //make the new list entry and add it to the end of the list
      new_desc->permisions = __open_fs_mask_to_u32int__(mask);
      new_desc->node = file;
      new_desc->next = 0;
      tmp_desc->next = new_desc;

      return new_desc;
    }else
      return 0;
  }else
    return 0;
}

u32int close_fs(FILE *file)
{
  if(file)
  {
    file_desc_t *tmp_desc;
    tmp_desc = initial_fdesc;

    //find our file descriptor
    for(; tmp_desc->next != file && tmp_desc; tmp_desc = tmp_desc->next);

    //a simple error check if the tmp_desc exists
    if(!tmp_desc)
      return 1;
      
    //remove the file descriptor entry
    tmp_desc->next = tmp_desc->next->next;

    kfree((void*)file);
    return 0;
  }
  else
    return 1;
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
      *(char*)(end + i) = *(char*)(start + i);

    memset((u32int*)(end + i), 0, -1 * shiftAmount);

    //success!
    return 0;
  }

}

int findOpenNode()
{
  int i;

  for(i = 0; i < nroot_nodes; i++)
    //if all of those headers are not used, then there is no file
    if(!*(root_nodes[i].name) && !root_nodes[i].inode && !root_nodes[i].mask && !root_nodes[i].flags)
      return i;

  nroot_nodes++;
  return nroot_nodes - 1;
}

u32int blockSizeAtIndex(u32int fileSize, u32int blockNum, u32int offset)
{
  if(blockNum * BLOCK_SIZE > fileSize)
    //error
    return 0;

  //num is the number of whole (full 1KB) blocks will fit in the file
  u32int num = (u32int)(fileSize / BLOCK_SIZE);

  //if the blockNum requested is less than the number of whole blocks, then it will fit BLOCK_SIZE
  if(blockNum < num)
  {
    if(!offset) //if there is no offset
      //default to the BLOCK_SIZE
      return BLOCK_SIZE;
    else{
      //if the offset exceeds the size of the block looked for
      if(offset > (blockNum + 1) * BLOCK_SIZE) 
        return 0; //error
      else{
        //if the offset is in the block looked for
        if(((blockNum + 1) * BLOCK_SIZE) - offset < BLOCK_SIZE) 
          //return the difference of the block size and the offset
          return ((blockNum + 1) * BLOCK_SIZE) - offset; 
        else
          return BLOCK_SIZE;
        
      }
    }

  }else if(blockNum == num)
  {
    if(!offset) //if there is no offset
      return fileSize % BLOCK_SIZE;
    else{
      //if the offset exceeds the size of the block looked for
      if(offset > (blockNum + 1) * BLOCK_SIZE) 
        return 0; //error!
      else{
        //if the offset is in the block looked for
        if(fileSize - offset < BLOCK_SIZE)
          return fileSize - offset;
        else //if the block looked for exceeds the block that the offset is in
          return fileSize % BLOCK_SIZE;

      }
    }

  }else if((blockNum + 1) * BLOCK_SIZE >= fileSize) //blockNum is too large and does not fir in the fileSize
    return 0;

}

//This may be difficult to understand due to lack of explaining comments
u32int *block_of_set(fs_node_t *node, u32int block_number)
{
  //if the block is a direct block
  if(block_number >= 0 && block_number < BLOCKS_DIRECT)
  {
    return &node->blocks[block_number];
    
  //if the block is in the singly set
  }else if(block_number >= BLOCKS_DIRECT && block_number < BLOCKS_DIRECT + BLOCKS_SINGLY)
  {
    //disregard the direct blocks
    u32int offset = block_number - BLOCKS_DIRECT;

    return (u32int*)(node->singly + offset);
    
  //if the block is in the doubly set
  }else if(block_number >= BLOCKS_DIRECT + BLOCKS_SINGLY &&
          block_number < BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY)
  {
    //disregard the direct blocks
    u32int offset = block_number - (BLOCKS_DIRECT + BLOCKS_SINGLY);

    /*the doubly_offset is the offset of the doubly block's node pointing towards the singly block,
     * the singly block offset is the offset of the singly block that is inside our doubly block that
     * points to our physical block*/
    u32int doubly_offset = (offset) / BLOCKS_SINGLY;
    u32int singly_offset = (offset) % BLOCKS_SINGLY;

    //if the singly pointer is 0, allocate space for it
    if(!*(node->doubly + doubly_offset))
      *(node->doubly + doubly_offset) = kmalloc(BLOCK_SIZE);

    //the value of the doubly returns a singly block, get the value of that and there is the direct block      
    u32int *doubly_pointer = (u32int*)*(node->doubly + doubly_offset);    
    u32int *singly_pointer = (u32int*)(doubly_pointer + singly_offset);

    //the singly_pointer points to the singly blocks pointer to the direct block's pointer
    return singly_pointer;

  //if the block is in the triply set
  }else if(block_number >= BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY &&
          block_number < BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY + BLOCKS_TRIPLY)
  {
    //disregard the direct blocks
    u32int offset = block_number - (BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY);

    /*trpily_offset is the offset in the triply block of the node pointing to the doubly block, the doubly_offset
     * is the offset inside the doubly block of the triply block that points to the singly block,
     * the singly_offset is the offset of the singly block inside the doubly block of the triply block
     * pointing to the block we want to return*/

    u32int triply_offset = (offset) / BLOCKS_DOUBLY;
    u32int doubly_offset = (offset % BLOCKS_DOUBLY) / BLOCKS_DOUBLY;
    u32int singly_offset = (offset % BLOCKS_DOUBLY) % BLOCKS_SINGLY;

    /*the value of the tripy block with an offset returns a doubly block,
     * the offset of that returns a singly block,
     * the offset of that gets the value of the desired block, return its memmory location*/
    u32int *triply_pointer = (u32int*)*(node->triply + triply_offset);
    u32int *doubly_pointer = (u32int*)*(triply_pointer + doubly_offset);
    u32int *singly_pointer = (u32int*)(doubly_pointer + singly_offset);

    //the singly_pointer points to the singly blocks pointer to the direct block's pointer
    return singly_pointer;

  }else{
    //the block number is not in range, error
    return 0;
  }
}

fs_node_t *vfs_createFile(fs_node_t *parentNode, char *name, u32int size)
{
  u32int n = findOpenNode();

  // Create a new file node.
  strcpy(root_nodes[n].name, name);
  root_nodes[n].mask = 0b110110100; //user rw, group rw, other r
  root_nodes[n].uid = root_nodes[n].gid = 0;
  root_nodes[n].inode = n;
  root_nodes[n].length = size;

  //set the pointers to singly, doubly, triply to 0
  root_nodes[n].singly = 0;
  root_nodes[n].doubly = 0;
  root_nodes[n].triply = 0;

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

    //if we need to alloc for the single, doubly, and triply
    if(i >= BLOCKS_DIRECT && i < BLOCKS_DIRECT + BLOCKS_SINGLY && !root_nodes[n].singly) 
    {
      //allocate for the singly block
      root_nodes[n].singly = (u32int*)kmalloc(BLOCK_SIZE);

      //clear any junk
      memset(root_nodes[n].singly, 0x0, BLOCK_SIZE);
    }else if(i >= BLOCKS_DIRECT + BLOCKS_SINGLY && 
            i < BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY && !root_nodes[n].doubly)
    {
      //allocate for the doubly block
      root_nodes[n].doubly = (u32int*)kmalloc(BLOCK_SIZE);

      //clear any junk
      memset(root_nodes[n].doubly, 0x0, BLOCK_SIZE);
    }else if(i >= BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY &&
            i < BLOCKS_DIRECT + BLOCKS_SINGLY + BLOCKS_DOUBLY + BLOCKS_TRIPLY && 
            !root_nodes[n].triply)
    {
      //allocate for the triply block
      root_nodes[n].triply = (u32int*)kmalloc(BLOCK_SIZE);

      //clear any junk
      memset(root_nodes[n].triply, 0x0, BLOCK_SIZE);
    }

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
    
    /*allocate space for a block, if it is a direct block, 
     * assign it to the node's structure, else assign it to
     * an indirect block's entry*/
    //~ if(i < BLOCKS_DIRECT)
      //~ *(u32int*)block = kmalloc(BLOCK_SIZE);
    //~ else
      //~ block = kmalloc(BLOCK_SIZE);

    *(u32int*)block = kmalloc(BLOCK_SIZE);
  }


  addFileToDir(parentNode, &root_nodes[n]);

  return &root_nodes[n];

}

int addHardLinkToDir(fs_node_t *hardlink, fs_node_t *directory, char *name)
{
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
      if(b * BLOCK_SIZE + i + dirent.rec_len >= directory->length)
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

fs_node_t *vfs_createDirectory(fs_node_t *parentNode, char *name)
{
  /*if the directory we are creating is not root, initrd_root is the
   * "mother root" directory, a directory that contains root and only root
   * directory, we can pass the '/' check for the root directory */
  if(parentNode != initrd_root)
  {
    
    //check if the dirName has any "/" as a dir name cannot contain them
    u32int i, length = strlen(name);
    for(i = 0; i < length; i++)
      if(*(name + i) == '/')
      {
        k_printf("The directory name: \"%s\" may not contain any \"/\" characters\n", name);
  
        //error!
        return 0;
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
    while(*(u16int*)(*(u32int*)block + i + sizeof(dirent.ino)))
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i += *(u16int*)(*(u32int*)block + i + sizeof(dirent.ino));

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a block can hold, exit and go to new block
      if(i + dirent.rec_len >= BLOCK_SIZE)
        break;

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a direcotory can hold, exit function before page fault happens
      if(b * BLOCK_SIZE + i + dirent.rec_len >= dirNode->length)
        //failed, out of directory left over space
        return 1;

    }
v
    //if i is a valid offset, do not go to a new block, just exit
    if(!*(u16int*)(*(u32int*)block + i + sizeof(dirent.ino)) == 0)
      break;

  }

  //assigns the contents of the struct dirent to the directory contents location
  memcpy((u32int*)(*(u32int*)block + i), &dirent, dirent.rec_len - dirent.name_len);

  strcpy((char*)(*(u32int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) 
                 + sizeof(dirent.name_len) + sizeof(dirent.file_type)), dirent.name);


  kfree(dirent.name);

  //success!
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

u32int vfs_remove_dirent(fs_node_t *directory, fs_node_t *node)
{
  struct dirent *dirent2;
  dirent2 = (struct dirent*)kmalloc(sizeof(struct dirent));

  u32int i = 0, b = 0, dir_block;
  //calculate the first dir_block
  dir_block = (u32int)block_of_set(directory, b);

  /*this section looks through the direcetory and finds the location
   * of the dirent of the file that we want to remove */
  do
  {

    //if we found the file in the directory's contents
    if(*(u32int*)(*(u32int*)dir_block + i) == node->inode)
    {

      dirent2->ino = *(u32int*)(*(u32int*)dir_block + i);
      dirent2->rec_len = *(u16int*)(*(u32int*)dir_block + i + sizeof(dirent2->ino));

      break;

    }else{

      //if the next rec_length is 0, then we are currently at the last dirent, else increase the offset (i)
      if(*(u16int*)(*(u32int*)dir_block + i + sizeof(dirent2->ino)))
        /*increase i with the rec_len that we get by moving fileheader sizeof(dirent2->ino)
         * (4 bytes) and reading its value*/
        i += *(u16int*)(*(u32int*)dir_block + i + sizeof(dirent2->ino));
      else{ //this is the last direct, add 1 to block and reset the offset (i)
        i = 0;
        b++;

        if(b > (u32int)((directory->length - 1) / BLOCK_SIZE))
          //error, block exceds the number of blocks dirNode has
          return 1;

        //recalculate the dir_block
        dir_block = (u32int)block_of_set(directory, b);
      }

    }
  }
  while(1);


  /*shifts the dirent data in the directory
   * the "-1 *" is used to show shift to the left */
  shiftData((u32int*)(*(u32int*)dir_block + i + dirent2->rec_len),
            -1 * dirent2->rec_len, BLOCK_SIZE - i - dirent2->rec_len);

  kfree(dirent2);

  //sucess!
  return 0;
}

u32int vfs_free_data_blocks(fs_node_t *directory, fs_node_t *node)
{
  if(!directory || !node)
    return 1; //error

  //free the actual node content
  u32int c, block, block_size = block_size_of_node(node);
  for(c = 0; c < (u32int)((node->length - 1) / block_size) + 1; c++)
  {
    block = (u32int)block_of_set(node, c);
    kfree((void*)(*(u32int*)block));
  }
    
  //sucess!
  return 0;

}
