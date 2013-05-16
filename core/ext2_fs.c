/*
 * ext2_fs.c
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

#include <system.h>

//defaults (logged as user) for permisions files need to have in order to be accesed
u32int _Rlogged = EXT2_I_RUSR, _Wlogged = EXT2_I_WUSR, _Xlogged = EXT2_I_XUSR;

u32int ext2_read(ext2_inode_t *node, u32int offset, u32int size, u8int *buffer)
{
  //if the user can read it
  if(node->mode & _Rlogged)
  {

  }
}

u32int ext2_write(ext2_inode_t *node, u32int offset, u32int size, u8int *buffer)
{
  //if the user can write to it
  if(node->mode & _Wlogged)
  {

  }
}

//TODO implement ext2_open
void ext2_open(ext2_inode_t *node, u8int read, u8int write)
{

}

//TODO implement ext2_close
void ext2_close(ext2_inode_t *node)
{

}

struct dirent *ext2_readdir(ext2_inode_t *node, u32int index)
{
  //Is the node a directory, and can it be accessed (executable)?
  if((node->mode & EXT2_I_DIR) && (node->mode & _Xlogged))
  {

  }

}

ext2_inode_t *ext2_finddir(ext2_inode_t *node, char *name)
{
  //Is the node a directory, and can it be accessed (executable)?
  if((node->mode & EXT2_I_DIR) && (node->mode & _Xlogged))
  {

  }
}

ext2_superblock_t *ext2_sblock_set_data()
{
  ext2_superblock_t *data;

  //only sets the data that is useful

  data->total_inodes;
  data->total_blocks;
  data->reserved_blocks;
  data->free_blocks;
  data->free_inodes;
  data->first_data_block;
  data->log2_block_size;
  data->log2_fragment_size;
  data->blocks_per_group;
  data->fragments_per_group;
  data->inodes_per_group;
  data->mtime;
  data->utime;
  data->mnt_count;
  data->max_mnt_count;
  data->magic = 0xef53;
  data->fs_state;
  data->error_handling;
  data->minor_revision_level;
  data->lastcheck;
  data->checkinterval;
  data->creator_os;
  data->revision_level;
  data->uid_reserved;
  data->gid_reserved;
  data->first_inode;
  data->inode_size;
  data->block_group_number;
  data->feature_compatibility;
  data->feature_incompat;
  data->feature_ro_compat;
  data->unique_id[4];
  data->volume_name[16];
  data->last_mounted_on[64];
  data->compression_info;

  return data;
}

ext2_group_descriptor_t *ext2_set_gdesc_table(u32int block_bitmap, u32int i_bitmap, u32int i_table)
{
  ext2_group_descriptor_t *ext2_table_data;

  //only sets the data that is useful

  ext2_table_data->block_bitmap = block_bitmap;
  ext2_table_data->inode_bitmap = i_bitmap;
  ext2_table_data->inode_table_id = i_table;
  ext2_table_data->free_blocks = FLOPPY_SIZE / EXT2_BLOCK_SZ;
  ext2_table_data->free_inodes = EXT2_I_PER_GRP;
  ext2_table_data->used_dirs = 0;
  ext2_table_data->pad;
  ext2_table_data->reserved[3];

  return ext2_table_data;
}

u32int ext2_set_block_group(u32int group_number)
{

  //write the super block to the floppy
  ext2_superblock_t *sblock_data;

  sblock_data = ext2_sblock_set_data();

  floppy_write((u32int*)sblock_data, sizeof(ext2_superblock_t), EXT2_SBLOCK_OFF % EXT2_BLOCK_SZ);

  //write the group block descriptors
  ext2_group_descriptor_t *gdesc_table_data;

  gdesc_table_data = ext2_set_gdesc_table((EXT2_SBLOCK_OFF + 2 * EXT2_BLOCK_SZ) % EXT2_BLOCK_SZ,
                                          (EXT2_SBLOCK_OFF + 3 * EXT2_BLOCK_SZ) % EXT2_BLOCK_SZ,
                                          (EXT2_SBLOCK_OFF + 4 * EXT2_BLOCK_SZ) % EXT2_BLOCK_SZ);

  floppy_write((u32int*)gdesc_table_data, sizeof(ext2_group_descriptor_t),
              (EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) % EXT2_BLOCK_SZ);

  //write zeros over the block and inode bitmaps
  u8int write_data = 0x0;
  u8int *mem;
  mem = (u8int*)kmalloc(2 * EXT2_BLOCK_SZ); //two blocks for the block and inode combined
  memset(mem, write_data, 2 * EXT2_BLOCK_SZ);

  floppy_write((u32int*)mem, 2 * EXT2_BLOCK_SZ,
                (EXT2_SBLOCK_OFF + 2 * EXT2_BLOCK_SZ) % EXT2_BLOCK_SZ);

  kfree(mem);

  //write zeros over the inode tables
  u8int *i_tables;
  i_tables = (u8int*)kmalloc(sizeof(ext2_inode_t) * EXT2_I_PER_GRP);
  memset(i_tables, write_data, sizeof(ext2_inode_t) * EXT2_I_PER_GRP);

  floppy_write((u32int*)i_tables, sizeof(ext2_inode_t) * EXT2_I_PER_GRP,
                (EXT2_SBLOCK_OFF + 4 * EXT2_BLOCK_SZ) % EXT2_BLOCK_SZ);

  kfree(i_tables);

  //sucess!
  return 0;
}
