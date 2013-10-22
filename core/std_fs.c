/*
 * std_fs.c
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

//the curent directory node
void *ptr_currentDir;

//initial file descriptor location
file_desc_t *initial_fdesc;

char *path; //a character array containing the path from root to the current directory

typedef struct generic_fs
{
  u8int magic;
} generic_fs_t;

file_desc_t *look_up_fdesc(void *node)
{
  file_desc_t *tmp_desc;
  tmp_desc = initial_fdesc;

  //find our file descriptor
  for(; tmp_desc->node != node && tmp_desc; tmp_desc = tmp_desc->next);

  //a simple error check if the tmp_desc exists
  if(!tmp_desc)
    return 0;

  return tmp_desc;
}

u32int f_read(FILE *node, u32int offset, u32int size, u8int *buffer)
{
  //check if this FILE node is in the file descriptor list
  file_desc_t *fdesc;
  fdesc = look_up_fdesc(node->node);

  //we did not find the file desc in the list
  if(!fdesc)
  {
    k_printf("Error: file not in file descriptor list\n");
    return 1; //error
  }

  //if the user can read from it
  if(fdesc->permisions & FDESC_READ)
  {
    switch(fdesc->fs_type)
    {
    case M_UNKNOWN:
      return 1; //error
    case M_VFS:
      //check if this node has a callback
      if(((fs_node_t*)fdesc->node)->read)
        return ((fs_node_t*)fdesc->node)->read(fdesc->node, offset, size, buffer);        
      else
        break;
    case M_EXT2:
      return ext2_read(fdesc->node, offset, size, buffer);
    default:
    }

  }else
    k_printf("Error: reading from an unprivilaged file\n");
  
  //if we have not exited yet, it is an error
  return 1;
}

u32int f_write(FILE *node, u32int offset, u32int size, u8int *buffer)
{
  //check if this FILE node is in the file descriptor list
  file_desc_t *fdesc;
  fdesc = look_up_fdesc(node->node);

  //we did not find the file desc in the list
  if(!fdesc)
  {
    k_printf("Error: file not in file descriptor list\n");
    return 1; //error
  }

  //if the user can read from it
  if(fdesc->permisions & FDESC_WRITE)
  {
    switch(fdesc->fs_type)
    {
    case M_UNKNOWN:
      return 1; //error
    case M_VFS:
      //check if this node has a callback
      if(((fs_node_t*)fdesc->node)->write)
        return ((fs_node_t*)fdesc->node)->write(fdesc->node, offset, size, buffer);        
      else
        break;
    case M_EXT2:
      return ext2_write(fdesc->node, offset, size, buffer);
    default:
    }

  }else
    k_printf("Error: writing to an unprivilaged file\n");
  
  //if we have not exited yet, it is an error
  return 1;
}

static u8int __open_fs_mask_to_u32int__(char *mask)
{
  if(!mask)
    return 0; //then there is no mask to return

  u8int flags = 0;
  u32int i;
  for(i = 0; i < strlen(mask); i++)
  {
    //assign the values of the flags
    switch(*(mask + i))
    {
      case 'd':
        flags |= FDESC_CLEAR;
        break;
      case 'r':
        flags |= FDESC_READ;
        break;
      case 'w':
        flags |= FDESC_WRITE;
        break;
      case 'a':
        flags |= FDESC_APPEND;
        break;
    }
  }

  return flags;
}

FILE *__open__(void *node, char *name, char *mask, u8int open)
{
  if(node)
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
      if(tmp_desc->node == node)
        return tmp_desc; //no need to open, just return it

    //make the new list entry and add it to the end of the list
    new_desc = (file_desc_t*)kmalloc(sizeof(file_desc_t));

    //copy the name over
    u32int name_len = strlen(name);
    new_desc->name = (char*)kmalloc(name_len + 1); // +1 for the \000
    memcpy(new_desc->name, name, name_len + 1);
    new_desc->name_len = name_len;

    new_desc->node = node;
    new_desc->fs_type = ((generic_fs_t*)node)->magic;
    new_desc->node_type = node_type(node);
    new_desc->permisions = __open_fs_mask_to_u32int__(mask);

    //set some file system specific elements
    switch(new_desc->fs_type)
    {
    case M_UNKNOWN:
      kfree(new_desc->name);
      kfree(new_desc);
      return 0; //error
    case M_VFS:
      new_desc->inode = ((fs_node_t*)node)->inode;
      new_desc->size = ((fs_node_t*)node)->length;

      //if it is a directory
      if(new_desc->node_type == TYPE_DIRECTORY)
      {
        new_desc->_read = 0;
        new_desc->_write = 0;
        new_desc->_finddir = finddir_fs;
        new_desc->_readdir = readdir_fs;
      }else{
        new_desc->_read = read_fs;
        new_desc->_write = write_fs;
        new_desc->_finddir = 0;
        new_desc->_readdir = 0;
      }

      break;
    case M_EXT2:
      new_desc->inode = ((ext2_inode_t*)node)->inode;
      new_desc->size = ((fs_node_t*)node)->size;

      //if it is a directory
      if(new_desc->node_type == TYPE_DIRECTORY)
      {
        new_desc->_read = 0;
        new_desc->_write = 0;
        new_desc->_finddir = ext2_file_from_dir;
        new_desc->_readdir = ext2_dirent_from_dir;
      }else{
        new_desc->_read = ext2_read;
        new_desc->_write = ext2_write;
        new_desc->_finddir = 0;
        new_desc->_readdir = 0;
      }

      break;
    default:
      kfree(new_desc->name);
      kfree(new_desc);
      return 0; //error
    }

    new_desc->next = 0;

    //add this file descriptor to the overall list, only if the user wants to
    if(open == TRUE)
      tmp_desc->next = new_desc;

    return new_desc;

  }else
    return 0;
}

FILE *f_open(char *filename, char *mask)
{
  FILE *file;
  file = f_finddir(ptr_currentDir, filename);

  //a file already exists to be opened
  if(file)
  {
    //if the user wants to write to a blank file
    if(__open_fs_mask_to_u32int__(mask) & (FDESC_CLEAR | FDESC_WRITE) == FDESC_CLEAR | FDESC_WRITE)
    {
      FILE *open = __open__(file->node, filename, mask, TRUE);
      
      //clear the contents of the file
      u8int *buf;
      buf = (u8int*)kmalloc(sizeof(u8int) * open->size);
      memset(buf, 0x0, open->size);
      f_write(open, 0, open->size, buf);

      kfree(buf);
      return open;
    }else
      return __open__(file->node, filename, mask, TRUE);
  }else //see if we can create a new file
    //the mask must have w set inorder to create the new file
    if(__open_fs_mask_to_u32int__(mask) & FDESC_WRITE)
      switch(((generic_fs_t*)ptr_currentDir)->magic)
      {
      case M_UNKNOWN:
        f_finddir_close(file);
        return 0; //error
      case M_VFS:
        f_finddir_close(file);
        return __open__(vfs_createFile(ptr_currentDir, filename, BLOCK_SIZE), filename, mask, TRUE);
      case M_EXT2:
        f_finddir_close(file);
        return __open__(ext2_create_file(ptr_currentDir, filename, BLOCK_SIZE), filename, mask, TRUE);
      default:
        f_finddir_close(file);
        return 0; //error
      }

  //if we are outside, return an error
  return 0;
}

u32int f_close(FILE *file)
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

    kfree(file->name);
    kfree((void*)file);
    return 0;
  }
  else
    return 1;
}

struct generic_dirent *f_readdir(void *node, u32int index)
{
  //is this node a directory
  if(node_type(node) == TYPE_DIRECTORY)
    switch(((generic_fs_t*)node)->magic)
    {
    case M_UNKNOWN:
      return 0; //error
    case M_VFS: 
      //if the node has a callback?
      if(((fs_node_t*)node)->readdir)
      {
        struct generic_dirent *gen_dirent;
        gen_dirent = ((fs_node_t*)node)->readdir(node, index);  

        //set the file type to the standard TYPE_
        switch(gen_dirent->file_type)
        {
        case FS_FILE:
          gen_dirent->file_type = TYPE_FILE;
          break;
        case FS_DIRECTORY:
          gen_dirent->file_type = TYPE_DIRECTORY;
          break;
        case FS_CHARDEVICE:
          gen_dirent->file_type = TYPE_CHARD_DEV;
          break;
        case FS_BLOCKDEVICE:
          gen_dirent->file_type = TYPE_BLOCK_DEV;
          break;
        case FS_PIPE:
          gen_dirent->file_type = TYPE_FIFO;
          break;
        case FS_SYMLINK:
          gen_dirent->file_type = TYPE_SYMLINK;
          break;
        case FS_MOUNTPOINT:
          gen_dirent->file_type = TYPE_MOUNTPOINT;
          break;
        default:
          gen_dirent->file_type = TYPE_UNKOWN;
          break;
        }

        return gen_dirent;

      }else
        break;
    case M_EXT2:
    { 
      struct generic_dirent *gen_dirent;     
      gen_dirent = ext2_dirent_from_dir(node, index);

      //set the file type to the standard TYPE_
      switch(gen_dirent->file_type)
      {
      case EXT2_UNKOWN:
        gen_dirent->file_type = TYPE_UNKOWN;
        break;
      case EXT2_FILE:
        gen_dirent->file_type = TYPE_FILE;
        break;
      case EXT2_DIR:
        gen_dirent->file_type = TYPE_DIRECTORY;
        break;
      case EXT2_CHARD_DEV:
        gen_dirent->file_type = TYPE_CHARD_DEV;
        break;
      case EXT2_BLOCK_DEV:
        gen_dirent->file_type = TYPE_BLOCK_DEV;
        break;
      case EXT2_FIFO:
        gen_dirent->file_type = TYPE_FIFO;
        break;
      case EXT2_SOCKET:
        gen_dirent->file_type = TYPE_SOCKET;
        break;        
      case EXT2_SYMLINK:
        gen_dirent->file_type = TYPE_SYMLINK;
        break;
      case EXT2_MOUNTPOINT:
        gen_dirent->file_type = TYPE_MOUNTPOINT;
        break;
      default:
        gen_dirent->file_type = TYPE_UNKOWN;
        break;
      }

      return gen_dirent;

    }
    default:
      return 0; //error
    
    }

  //if we are outside, regard it as an error
  return 0; //error
}

void f_finddir_close(FILE *node)
{
  if(look_up_fdesc(node->node))
    return; //error, we do not want to close an opened file descriptor

  //simply free the node and its name
  kfree(node->name);
  kfree(node);

  return;
}

FILE *f_finddir(void *node, char *name)
{
  //is this node a directory
  if(node_type(node) == TYPE_DIRECTORY)
  {
    //if name is input 0, then we should find the name of the node
    if(!name)
      name = name_of_dir(node);

    switch(((generic_fs_t*)node)->magic)
    {
    case M_UNKNOWN:
      return 0; //error
    case M_VFS: 
    {
      //case the void * node to the vfs node structure
      fs_node_t *vfs_node = node;

      //if the node has a callback?
      if(vfs_node->finddir)
        //return an unopened file node with no r/w/a permissions at all to the actuall node data
        return __open__(vfs_node->finddir(vfs_node, name), name, 0, FALSE);
      else
        break;
    }
    case M_EXT2:
      //return an unopened file node with no r/w/a permissions at all to the actuall node data
      return __open__(ext2_file_from_dir(node, name), name, 0, FALSE);
    default:
      return 0; //error
    
    }

  }

  //if we are outside, regard it as an error
  return 0; //error
}

u32int setCurrentDir(void *directory)
{
  kfree(path); //frees the contents of the char array pointer, path

  ptr_currentDir = directory; //sets the value of the dir inode to the cuurentDir_inode

  void *node = directory;
  void *copy;

  u32int count = 0, totalCharLen = 0, allocated_names = 15, tmp_name_len;

  char **name_locs, *tmp_name;
  //where we will store
  name_locs = (char**)kmalloc(allocated_names * sizeof(char*));

  /*starts from the current directory, goes backwards by getting the node
   * of the parent (looking up the data in ".." dir), adding its namelen
   * to the u32int totalCharLen and getting its parent, etc.
   *
   *once the parent is the same as the child, that only occurs with the
   * root directory, so we should exit */
  do
  {
    copy = node;

    if(copy)
    {
      tmp_name = name_of_dir(copy);
      tmp_name_len = strlen(tmp_name);

      /*save the name now so we do not need to do this all over again
       * when concatonating all of the names into one path name*/
      *(name_locs + count) = (char*)kmalloc(tmp_name_len + 1); //+1 for the \000     
      memcpy(*(name_locs + count), tmp_name, tmp_name_len);
      *(*(name_locs + count) + tmp_name_len) = 0; //add the \000 to the end

      //+1 being the preceding "/" to every dir that will be added later
      totalCharLen = totalCharLen + tmp_name_len + 1; 
      count++;
    }

    //we have run out of space in our names space, realloc more space!
    if(count == allocated_names)
      krealloc(name_locs, sizeof(char*) * allocated_names, sizeof(char*) * (allocated_names *= 2))

    node = f_finddir(copy, "..");
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

    char *__name_of_copy__;
    u32int i, charsWritten = 0, nameLen;
    for(i = 0; i < count; i++)
    {
      __name_of_copy__ = *(name_locs + i);
      nameLen = strlen(__name_of_copy__);

      /* i < count - 2 is a protection from drawing the preceding "/"
       * on the first two dirs in the path (root and one more). The first two dirs will
       * allways be drawn the last two times (we write the dir names to path from
       * current dir (top) to root (bottom)), thus if i is less
       * than the count - 2, that means we are not yet at the last
       * two drawing and it is ok to have a precedding "/" */
      if(strcmp(__name_of_copy__, fs_root->name) && i < count - 2)
      {
        memcpy(path + (totalCharLen - charsWritten - nameLen - 1), "/", 1);
        memcpy(path + (totalCharLen - charsWritten - nameLen), __name_of_copy__, nameLen);
        charsWritten = charsWritten + nameLen + 1; //increment charsWritten with the "/<name>" string we just wrote, +1 is that "/"

      }else{
        memcpy(path + (totalCharLen - charsWritten - nameLen), __name_of_copy__, nameLen);
        charsWritten = charsWritten + nameLen; //increment charsWritten with the "/" string we just wrote
      }

    }

    *(path + totalCharLen) = 0; //added \000 to the end of path

  }else{
    //keep it simple, if root is the only directory, copy its name manually
    path = (char*)kmalloc(2); //2 chars beign "/" for root and \000

    *(path) = '/';
    *(path + 1) = 0; //added \000 to the end
  }
  
  //sucess!
  return 0;
}

u32int node_type(void *node)
{
  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return TYPE_UNKOWN; //error
  case M_VFS:
    switch(((fs_node_t*)node)->flags)
    {
    case FS_FILE:
      return TYPE_FILE;
    case FS_DIRECTORY:
      return TYPE_DIRECTORY;
    case FS_CHARDEVICE:
      return TYPE_CHARD_DEV;
    case FS_BLOCKDEVICE:
      return TYPE_BLOCK_DEV;
    case FS_PIPE:
      return TYPE_FIFO;
    case FS_SYMLINK:
      return TYPE_SYMLINK;
    case FS_MOUNTPOINT:
      return TYPE_MOUNTPOINT;
    default:
      return TYPE_UNKOWN;
    }
  case M_EXT2:
    switch(((ext2_inode_t*)node)->type)
    {
    case EXT2_FILE:
      return TYPE_FILE;
    case EXT2_DIR:
      return TYPE_DIRECTORY;
    case EXT2_CHARD_DEV:
      return TYPE_CHARD_DEV;
    case EXT2_BLOCK_SZ:
      return TYPE_BLOCK_DEV;
    case EXT2_FIFO:
      return TYPE_FIFO;
    case EXT2_SYMLINK:
      return TYPE_SYMLINK;
    case EXT2_MOUNTPOINT:
      return TYPE_MOUNTPOINT;
    default:
      return TYPE_UNKOWN;
    }
  default:
    return TYPE_UNKOWN; //error
    
  }
  
  //if we are outside, that is an error
  return TYPE_UNKOWN;
}

char *name_of_dir(void *node)
{
  if(node_type(node) == TYPE_DIRECTORY)
  {
    switch(((generic_fs_t*)node)->magic)
    {
    case M_UNKNOWN:
      return 0; //error
    case M_VFS:
      return ((fs_node_t*)node)->name;
    case M_EXT2:
      return ext2_get_name_of_dir(node);
    default:
      return 0; //error
    }
  }else
    return 0; //error
}

u32int block_size_of_node(FILE *node)
{
  switch(node->fs_type)
  {
  case M_UNKNOWN:
    return 0; //error
  case M_VFS:
    return BLOCK_SIZE; //the vfs block size
  case M_EXT2:
    return EXT2_BLOCK_SZ; //the ext2 block size
  default:
    return 0; //error
  }
    
}

u32int f_remove(void *dir, FILE *node)
{

  switch(((generic_fs_t*)dir)->magic)
  {
  case M_UNKNOWN:
    return 1; //error
  case M_VFS:    
    //remove the node's dirent from dir
    if(__remove_dirent__(dir, node))
      return 1; //error

    //remove the node's dirent from dir
    if(__free_data__(dir, node))
      return 1; //error

    return 0;
  case M_EXT2:
    return ext2_delete(dir, node->node);
  default:
    return 1; //error
  }
  
}

u32int __free_data__(void *dir, FILE *node)
{
  switch(((generic_fs_t*)dir)->magic)
  {
  case M_UNKNOWN:
    return 1; //error
  case M_VFS:
    if(node->fs_type != M_VFS)
      return 1; //error, the node is not the same filesystem type as the directory

    return fs_free_data_blocks(dir, node->node);
  case M_EXT2:
    if(node->fs_type != M_EXT2)
      return 1; //error, the node is not the same filesystem type as the directory
    
    return ext2_free_data_blocks(dir, node->node);
  default:
    return 1; //error
  }
}

u32int __remove_dirent__(void *dir, FILE *dirent_node)
{ 
  switch(((generic_fs_t*)dir)->magic)
  {
  case M_UNKNOWN:
    return 1; //error
  case M_VFS:
    if(dirent_node->fs_type != M_VFS)
      return 1; //error, the node is not the same filesystem type as the directory

    return fs_remove_dirent(dir, dirent_node->node);
  case M_EXT2:
    if(dirent_node->fs_type != M_EXT2)
      return 1; //error, the node is not the same filesystem type as the directory
    
    return ext2_remove_dirent(dir, dirent_node->node);
  default:
    return 1; //error
    
  }
  
}

void *f_make_dir(void *dir, char *name)
{
  if(!name)
    return 1;

  switch(((generic_fs_t*)dir)->magic)
  {
  case M_UNKNOWN:
    return 1; //error
  case M_VFS:
    return vfs_createDirectory(dir, name);
  case M_EXT2:
    return ext2_create_dir(dir, name);
  default:
    return 1; //error
    
  }  
}
