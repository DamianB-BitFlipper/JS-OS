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

u32int f_read(void *node, u32int offset, u32int size, u8int *buffer)
{
  file_desc_t *fdesc;
  fdesc = look_up_fdesc(node);

  //we did not find the file desc in the list
  if(!fdesc)
  {
    k_printf("Error: file not in file descriptor list\n");
    return 1; //error
  }

  //if the user can read from it
  if(fdesc->permisions & FDESC_READ)
    return node->read(node, offset, size, buffer);
  else
    k_printf("Error: reading from an unprivilaged file\n");
  
  //if we have not exited yet, it is an error
  return 1;
}

u32int f_write(void *node, u32int offset, u32int size, u8int *buffer)
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

static u8int __open_fs_mask_to_u32int__(char *mask)
{
  u8int flags = 0;
  u32int i;
  for(i = 0; i < strlen(mask); i++)
  {
    //assign the values of the flags
    switch(*(mask + i))
    {
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

FILE *f_open(char *filename, char *mask)
{
  void *file;
  file = finddir_fs(ptr_currentDir, filename);

  if(file)
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

    //make the new list entry and add it to the end of the list
    new_desc = (file_desc_t*)kmalloc(sizeof(file_desc_t));

    //copy the name over
    u32int name_len = strlen(filename);
    new_desc->name = (char*)kmalloc(name_len + 1); // +1 for the \000
    memcpy(new_desc->name, filename, name_len + 1);
    new_desc->name_len = name_len;

    new_desc->node = file;
    new_desc->fs_type = ((generic_fs_t*)file)->magic;
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
      new_desc->inode = ((fs_node_t*)file)->inode;
      new_desc->size = ((fs_node_t*)file)->length;

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
      new_desc->inode = ((ext2_inode_t*)file)->inode;
      new_desc->size = ((fs_node_t*)file)->size;

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

    //add this file descriptor to the overall list
    tmp_desc->next = new_desc;

    return new_desc;

  }else
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

struct dirent *f_readdir(void *node, u32int index)
{
  // Is the node a directory, and does it have a callback?
  if((node->flags & 0x7) == FS_DIRECTORY && node->readdir != 0)
    return node->readdir(node, index);
  else
    return 0;
}

void *f_finddir(void *node, char *name)
{
  // Is the node a directory, and does it have a callback?
  if((node->flags & 0x7) == FS_DIRECTORY && node->finddir != 0)
    return node->finddir(node, name);
  else
    return 0;
}

u32int setCurrentDir(void *node)
{
  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return 1; //error
  case M_VFS:
    if(vfs_setCurrentDir((fs_node_t*)node))
      return 1; //error
    else
      return 0; //sucess!
  case M_EXT2:
    if(ext2_set_current_dir((ext2_inode_t*)node))
      return 1; //error
    else
      return 0; //sucess!
  default:
    return 1; //error
    
  }

  //if we are outside, that is an error
  return 1;
}

u32int node_type(void *node)
{
  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return 0; //error
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
