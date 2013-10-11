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

file_desc_t *initial_fdesc;

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

u32int read_fs(void *node, u32int offset, u32int size, u8int *buffer)
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
    switch(fdesc->fs_type)
    {
    case M_EXT2:
      return ext2_read(node, offset, size, buffer);
    case M_VFS:
      return ((fs_node_t*)node)->read(node, offset, size, buffer);
    default:
      k_printf("Error: attempting to read from an unknow file system type\n");
      return 1; //error
    }

  else
    k_printf("Error: reading from an unprivilaged file\n");
  
  //if we have not exited yet, it is an error
  return 1;
}

u32int write_fs(void *node, u32int offset, u32int size, u8int *buffer)
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
  if(fdesc->permisions & FDESC_WRITE)
    switch(fdesc->fs_type)
    {
    case M_EXT2:
      return ext2_write(node, offset, size, buffer);
    case M_VFS:
      return ((fs_node_t*)node)->write(node, offset, size, buffer);
    default:
      k_printf("Error: attempting to write to an unknow file system type\n");
      return 1; //error
    }

  else
    k_printf("Error: writing to an unprivilaged file\n");
  
  //if we have not exited yet, it is an error
  return 1;
}

u8int __open_fs_mask_to_u32int__(char *mask)
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

FILE *open_fs(char *filename, void *dir, char *mask)
{
  void *file;
  file = finddir_fs(dir, filename);

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

    new_desc = (file_desc_t*)kmalloc(sizeof(file_desc_t));
      
    //make the new list entry and add it to the end of the list
    new_desc->permisions = __open_fs_mask_to_u32int__(mask);
    new_desc->node = file;
    new_desc->next = 0;
    tmp_desc->next = new_desc;

    return new_desc;
  }

  //if were are outside, return an error
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

void *readdir_fs(void *node, u32int index)
{

  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return 0; //error
  case M_VFS:
    // Is the node a directory, and does it have a callback?
    if((((fs_node_t*)node)->flags & 0x7) == FS_DIRECTORY && ((fs_node_t*)node)->readdir)
      return ((fs_node_t*)node)->readdir(node, index);
  case M_EXT2:
  {
    struct ext2_dirent *dirent;
    if(!(dirent = ext2_dirent_from_dir(node, index)))
      return 0; //error
    else
      return dirent;
  }
  default:
    return 0; //error
    
  }

  //if we are ouside the switch, it must be an error
  return 0;
}

void *finddir_fs(void *node, char *name)
{
  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return 0; //error
  case M_VFS:
    // Is the node a directory, and does it have a callback?
    if((((fs_node_t*)node)->flags & 0x7) == FS_DIRECTORY && ((fs_node_t*)node)->finddir)
      return ((fs_node_t*)node)->finddir(node, name);
    else
      return 0; //error
  case M_EXT2:
  {
    ext2_inode_t *file;
    if(!(file = ext2_file_from_dir(node, name)))
      return 0; //error
    else
      return file;
  }
  default:
    return 0; //error
    
  }
 
  //if we are outside, that is an error
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

char *name_of_node(void *parent_dir, void *node)
{
  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return 0; //error
  case M_VFS:
    return ((fs_node_t*)node)->name;
  case M_EXT2:
    return ext2_name_of_node(parent_dir, node);
  default:
    return 0; //error
    
  }

  //if we are outside, that is an error
  return 1;
}

u32int type_of_node(void *node)
{
  switch(((generic_fs_t*)node)->magic)
  {
  case M_UNKNOWN:
    return 1; //error
  case M_VFS:
  {
    u32int mask = ((fs_node_t*)node)->mask;
    switch(mask)
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
    case FS_HARDLINK:
      return TYPE_HARDLINK;
    case FS_MOUNTPOINT:
      return TYPE_MOUNTPOINT;
    default:
      return TYPE_UNKOWN;
    }
  }case M_EXT2:
  {
    u32int mask = ((ext2_inode_t*)node)->type;
    switch(mask)
    {
    case EXT2_UNKOWN:
      return TYPE_UNKOWN;
    case EXT2_FILE:
      return TYPE_FILE;
    case EXT2_DIR:
      return TYPE_DIRECTORY;
    case EXT2_CHARD_DEV:
      return TYPE_CHARD_DEV;
    case EXT2_BLOCK_DEV:
      return TYPE_BLOCK_DEV;
    case EXT2_FIFO:
      return TYPE_FIFO;
    case EXT2_SOCKET:
      return TYPE_SOCKET;
    case EXT2_SYMLINK:
      return TYPE_SYMLINK;
    case EXT2_HARDLINK:
      return TYPE_HARDLINK;
    case EXT2_MOUNTPOINT:
      return TYPE_MOUNTPOINT;
    default:
      return TYPE_UNKOWN;
    }    
  }
    
  }
 
}
