/*
 * ext2_fs.c
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

//patch is a variable that offsets all writings to the floppy, when the fdc driver can write to sector 3
u32int patch = 0;

//defaults (logged as user) for permisions files need to have in order to be accesed
u32int _Rlogged = EXT2_I_RUSR, _Wlogged = EXT2_I_WUSR, _Xlogged = EXT2_I_XUSR;
u32int inode_table_size = 0;

u32int ext2_read(ext2_inode_t *node, u32int offset, u32int size, u8int *buffer)
{
  //if the user can read it
  if(node->mode & _Rlogged)
  {

  }
}

u32int ext2_read_meta_data(ext2_superblock_t **sblock, ext2_group_descriptor_t **gdesc)
{
  ext2_superblock_t *sdata;
  
  //get the sblock data
  sdata = (ext2_superblock_t*)kmalloc(sizeof(ext2_superblock_t));
  floppy_read((u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE), sizeof(ext2_superblock_t), (u32int*)sdata);

  u32int u;
  for(u = 0; u < 30; u++)
    k_printf("%h ", *((u8int*)sdata + u));

  k_printf("BLOCKS PER GROUOPS: %d\n", sdata->blocks_per_group);

  u32int nblockgroups = sdata->total_blocks / sdata->blocks_per_group;

  ext2_group_descriptor_t *gdata;

  //get the group descriptor data
  gdata = (ext2_group_descriptor_t*)kmalloc(nblockgroups * sizeof(ext2_group_descriptor_t));
  floppy_read((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / SECTOR_SIZE, nblockgroups * sizeof(ext2_group_descriptor_t), (u32int*)gdata);

  *sblock = sdata;
  k_printf("junk location %h, actual %h\n", *sblock, sdata);

  *gdesc = gdata;

  //Sucess!
  return 0;
}

u32int *ext2_format_block_bitmap(u32int location, u32int blocks_used)
{
  u8int *block_bitmap;
  block_bitmap = (u8int*)kmalloc(EXT2_BLOCK_SZ);
  floppy_read(location, EXT2_BLOCK_SZ, (u32int*)block_bitmap);
    
  u32int *output;
  output = (u32int*)kmalloc(blocks_used * sizeof(u32int));

  memset(output, 0x0, blocks_used * sizeof(u32int));
  
  //the number of consecutive blocks free
  u32int consec_free = 0;
  s32int bit_off;

  u32int off, bit_mask;
  for(off = 0; off < EXT2_BLOCK_SZ; off++)
  {
    //go through all bit mask starting from 0b10000000 to 0b1 inclusive
    for(bit_mask = 0b10000000, bit_off = 0; bit_mask >= 0b1; bit_mask >>= 1, bit_off++)
    {
      //if the bit isolated by bit_mask is 0
      if(!(*(block_bitmap + off) & bit_mask))
      {
        consec_free++;
      }else{ //if the bit isolated by bit_mask is 1
        consec_free = 0;
      }

      //we have found adequate space
      if(consec_free == blocks_used)
      {

        //offset output to the end
        output += blocks_used;

        //going backwards
        for(consec_free; consec_free > 0; consec_free--)
        {
          //flip the current bit and go backwards
          *(block_bitmap + off) |= bit_mask;

          output--;
          //assign the offset in bits from the beginning
          *output = 8 * off + bit_off;

          //decrement everything by one
          bit_mask <<= 1;
          bit_off--;

          //if bit_mask > 0b10000000, then reset bit_mask, bit_off, and move the off back one byte
          if(bit_mask > 0b10000000)
          {
            bit_mask = 0b1;
            bit_off = 7;
            off--;
          }
        }

        floppy_write((u32int*)block_bitmap, EXT2_BLOCK_SZ, location);

        kfree(block_bitmap);
        return output;
      }
    }

  }

  kfree(block_bitmap);

  //if we did not exit yet, there must be no space
  return 0;
}

u32int ext2_read_data_block(u32int number)
{

}

u32int ext2_inode_entry_blocks(ext2_inode_t *inode, ext2_group_descriptor_t *gdesc, u32int *block_locations, u32int blocks_used)
{
  u32int blk;
  //write the locations of the direct blocks
  for(blk = 0; blk < (blocks_used < EXT2_NDIR_BLOCKS ? blocks_used : EXT2_NDIR_BLOCKS); blk++)
    inode->blocks[blk] = *(block_locations + blk);

  if(blocks_used < EXT2_NDIR_BLOCKS)
  {
    //we already wrote all of the data needed to be written above, return with sucess
    return 0;
  }else{
    u32int *location;
    
    //subtract the direct blocks that we have accounted for
    blocks_used -= EXT2_NDIR_BLOCKS;

    location = ext2_format_block_bitmap(gdesc->block_bitmap, 1);
    
    //write the locations of the singly indirect blocks
    for(blk = 0; blk < (blocks_used < EXT2_NIND_BLOCK ? blocks_used : EXT2_NIND_BLOCK); blk++)
      inode->blocks[blk] = *(block_locations + blk);

    if(blocks_used > EXT2_NIND_BLOCK)
    {
      //subtract the singly indirect blocks that we have accounted for
      blocks_used -= EXT2_NIND_BLOCK;

      //write the locations of the doubly indirect blocks
      for(blk = 0; blk < (blocks_used < EXT2_NDIND_BLOCK ? blocks_used : EXT2_NDIND_BLOCK); blk++)
        inode->blocks[blk] = *(block_locations + blk);

      if(blocks_used > EXT2_NDIND_BLOCK)
      {
        //subtract the singly indirect blocks that we have accounted for
        blocks_used -= EXT2_NTIND_BLOCK;
  
        //write the locations of the triply indirect blocks
        for(blk = 0; blk < (blocks_used < EXT2_NTIND_BLOCK ? blocks_used : EXT2_NTIND_BLOCK); blk++)
          inode->blocks[blk] = *(block_locations + blk);
      }
    }

    kfree(location);

  }
}

ext2_inode_t *ext2_create_file(ext2_inode_t *parentNode, char *name, u32int size)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;
  
  ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);

  k_printf("sblock total blocks = %d, gdesc: BB %d\n", (u32int)(sblock->total_blocks / sblock->blocks_per_group), gdesc->block_bitmap);

  u32int *block_locations;

  //the the number of blocks the initial size will take up
  u32int blocks_used = (u32int)((size - 1) / EXT2_BLOCK_SZ) + 1;
  block_locations = ext2_format_block_bitmap(gdesc->block_bitmap, blocks_used);
  
  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

  data->mode = EXT2_I_RUSR | EXT2_I_WUSR | EXT2_I_RGRP | EXT2_I_WGRP | EXT2_I_ROTH | EXT2_I_WOTH;
  data->uid = 0;
  data->size = size;
  data->atime = posix_time();
  data->ctime = posix_time();
  data->mtime = posix_time();
  data->dtime = 0;
  data->gid = 0;
  data->nlinks = 1;    //number of hard links
  data->nblocks = 0;   //blocks of 512 bytes
  data->flags = 0;
  data->osd1 = EXT2_OS_JSOS;

  ext2_inode_entry_blocks(data, gdesc, block_locations, blocks_used);
  
  data->version = 0;
  data->fire_acl = 0;
  data->dir_acl = 0;
  data->fragment_addr = 0;

  kfree(sblock);
  kfree(gdesc);
  kfree(block_locations);
  kfree(data);
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

void ext2_sblock_set_data(ext2_superblock_t *data, u32int reserved_blocks, u32int sblock_location,
                          u32int error_handling, u32int sblock_group_num, char *partition_name)
{
  data->total_inodes = 0;
  data->total_blocks = 0;
  data->reserved_blocks = reserved_blocks;
  data->first_data_block = sblock_location;
  data->block_size = EXT2_BLOCK_SZ;
  data->blocks_per_group = 0;
  data->mtime = 0;
  data->wtime = 0;
  data->utime = 0;
  data->mnt_count = 0;
  data->max_mnt_count = EXT2_MAX_MNT_COUNT;
  data->magic = EXT2_MAGIC;
  data->fs_state = EXT2_VALID_FS;            //we are creating the fs, everything should be ok
  data->error_handling = error_handling;
  data->minor_revision_level = 0;
  data->lastcheck = 0;
  data->checkinterval = EXT2_CHECK_INTERVAL;
  data->creator_os = EXT2_OS_JSOS;           //we have created this fs on JS-OS
  data->revision_level = EXT2_GOOD_OLD_REV;
  data->first_inode = 0;
  data->inode_struct_max = sizeof(ext2_inode_t);
  data->block_group_number = sblock_group_num;
  data->feature_compatibility = 0;           //for any special features, none as of now
  data->feature_incompat = 0;
  data->feature_ro_compat = 0;

  //setting the unique id
  *(data->unique_id) = 0;
  *(data->unique_id + 1) = 0;
  *(data->unique_id + 2) = 0;
  *(data->unique_id + 3) = 0;

  //add the null terminating 0
  if(strlen(partition_name) > (sizeof(data->volume_name) - 1)) //if partition name is greater than the space for it
    *(data->volume_name + (sizeof(data->volume_name) - 1)) = 0; //cut it off at the end with a \000
  else
    *(data->volume_name + strlen(partition_name)) = 0; //else just add a 0 to the end of partition_name

  strcpy(data->volume_name, partition_name);

  memset(data->last_mounted_on, 0, sizeof(data->last_mounted_on));
  data->compression_info = EXT2_NO_ALG;

  //sucess!
  return;
}

void ext2_set_gdesc_table(ext2_group_descriptor_t *data)
{
  //just sets all contents to null

  data->block_bitmap = 0;
  data->inode_bitmap = 0;
  data->inode_table_id = 0;
  data->free_blocks = 0;
  data->free_inodes = 0;
  data->used_dirs = 0;
  data->pad = 0;

  u32int i;
  for(i = 0; i < 3; i++)
    *(data->reserved + i) = 0;

}

u32int ext2_set_block_group(u32int size)
{
  //save the original size of the partition
  u32int orig_size = size;

  //we make u32int size equal the block size in bytes
  size = size > (EXT2_MAX_BLK_GRP_SZ * EXT2_BLOCK_SZ) ? (EXT2_MAX_BLK_GRP_SZ * EXT2_BLOCK_SZ) : size;
  u32int blocks_per_group = (u32int)(size / EXT2_BLOCK_SZ);
  u32int inodes_per_group = size == (EXT2_MAX_BLK_GRP_SZ * EXT2_BLOCK_SZ) ? EXT2_I_PER_GRP : EXT2_I_PER_GRP_FLPPY;
  inode_table_size = (inodes_per_group * sizeof(ext2_inode_t)) / EXT2_BLOCK_SZ;
  u32int nblock_groups = (u32int)(orig_size / size);

  //super block data
  ext2_superblock_t *sblock_data;
  sblock_data = (ext2_superblock_t*)kmalloc(sizeof(ext2_superblock_t));

  //group descriptor data
  ext2_group_descriptor_t *gdesc_table_data;
  //one group desciptor data
  gdesc_table_data = (ext2_group_descriptor_t*)kmalloc(sizeof(ext2_group_descriptor_t));

  //the buffer that will hold all of the group descriptors
  u8int *gdesc_buf;
  gdesc_buf = (u8int*)kmalloc(nblock_groups * EXT2_BLOCK_SZ);
  
  //block bitmap, inode bitmap, inode table locations
  u32int block_BB, block_IB, block_IT;
  u8int write_data = 0x0;

  //block bitmap and inode bitmap data
  u8int *mem;
  mem = (u8int*)kmalloc(2 * EXT2_BLOCK_SZ); //two blocks for the block bitmap and inode bitmap combined
  memset(mem, write_data, 2 * EXT2_BLOCK_SZ);

  //inode table data
  u8int *i_tables;
  i_tables = (u8int*)kmalloc(sizeof(ext2_inode_t) * inodes_per_group);
  memset(i_tables, write_data, sizeof(ext2_inode_t) * inodes_per_group);

  u32int nbgroup, group_offset;
  for(nbgroup = 0; nbgroup < nblock_groups; nbgroup++)
  {
    //group offset changes so the floppy_writes write to group 1, 2, 3... as nbgroup increments
    group_offset = nbgroup * blocks_per_group;
    
    //!write the primary super block to the floppy (SB)
    ext2_sblock_set_data(sblock_data, 0,
                          group_offset + (u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE) + patch,
                          EXT2_ERRORS_CONTINUE, 0, "Primary");
    sblock_data->total_blocks = (u32int)(orig_size / EXT2_BLOCK_SZ);
    sblock_data->blocks_per_group = (u32int)(blocks_per_group);
    sblock_data->block_group_number = nbgroup; //the block group number that this superblock is located in
    
    floppy_write((u32int*)sblock_data, sizeof(ext2_superblock_t),
                  group_offset + (u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE) + patch); //write to sector 2

    //!write zeros over the block bitmap and inode bitmap (BB + IB)
    /*Pertaining to block bitmap, inode bitmap, inode table:
     * 
     * group_offset is the offset that says in what group to write the data in, 0, 1, 2, 3... etc
     * (u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / EXT2_BLOCK_SZ) + nblock_groups is the offset from the super block and group descriptor
     * the [(u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / EXT2_BLOCK_SZ] part is from the superblock,
     * which ends 2 blocks from the begining of the floppy
     * + nblock_groups is the offset that the group descriptor has as it takes
     * up the (number of block groups) blocks, i.e. if there are 2 block groups, the group descriptor would take up 2 blocks
     *
     * the multiplication in front of the first EXT2_BLOCK_SZ is an additional offset
     * for example, the inode bitmap is 1 block after the block bitmap, so another EXT2_BLOCK_SZ offset is necessary*/
    block_BB = group_offset + (u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE) + patch;
    block_IB = group_offset + (u32int)((EXT2_SBLOCK_OFF + 2 * EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE) + patch;

    k_printf("BB %d IB %d\n", block_BB, block_IB);
  
    floppy_write((u32int*)mem, 2 * EXT2_BLOCK_SZ, block_BB); //write to sector 3 and 5
  
    //!write zeros over the inode tables (IT)
    block_IT = group_offset + (u32int)((EXT2_SBLOCK_OFF + 3 * EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE) + patch;
    
    floppy_write((u32int*)i_tables, sizeof(ext2_inode_t) * inodes_per_group, block_IT); //write to sector 5
    //~ k_printf("IT size: %d, %h, loc: %d\n", inode_table_size, sizeof(ext2_inode_t) * inodes_per_group, block_IT);
  
    u32int gdesc;
    for(gdesc = 0; gdesc < nblock_groups; gdesc++)
    {
      //!write the group block descriptors (GBD)
      ext2_set_gdesc_table(gdesc_table_data);
  
      //Now that I have the block numbers for the block bitmap, inode bitmap, and inode tables, add them to group descriptor
      gdesc_table_data->block_bitmap = block_BB;
      gdesc_table_data->inode_bitmap = block_IB;
      gdesc_table_data->inode_table_id = block_IT;

      /*takes the blocks in one group, subtracts 3 for the super block, block bitmap, and inode bitmap
       * then subtracts the size of the inode table for this partition
       * and subtracts the number of block groups there are, since for every block group the is, 1 block is added to the descriptor tables */
      gdesc_table_data->free_blocks = blocks_per_group - (3 + inode_table_size + nblock_groups);
      
      gdesc_table_data->free_inodes = inodes_per_group;

      //write the actual contents in the beggining of the EXT2_BLOCK_SZ of the buffer
      memcpy((u32int*)(gdesc_buf + gdesc * SECTOR_SIZE), (u32int*)gdesc_table_data, sizeof(ext2_group_descriptor_t));

      //fill the rest of the EXT2_BLOCK_SZ of the buffer that was not used with zeros
      memset((u32int*)(gdesc_buf + gdesc * SECTOR_SIZE + sizeof(ext2_group_descriptor_t)), 0x0, EXT2_BLOCK_SZ - sizeof(ext2_group_descriptor_t));
      
    }

    floppy_write((u32int*)gdesc_buf, nblock_groups * SECTOR_SIZE,
                group_offset + (u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / SECTOR_SIZE) + patch); //write starting from sector 2
  }
  
  kfree(mem);
  kfree(i_tables);

  kfree(sblock_data);
  kfree(gdesc_table_data);
  kfree(gdesc_buf);

  //sucess!
  return 0;
}

void *ext2_edit_block_headers(u32int header, u32int block_group)
{
  switch(header)
  {
    case 0: //write to superblock
    {
      //get lba of first sblock
      u32int lba = (u32int)((EXT2_SBLOCK_OFF) / EXT2_BLOCK_SZ) + patch;
      
      ext2_superblock_t *buffer;
      floppy_read(lba, EXT2_BLOCK_SZ, (u32int*)buffer);

      //if block_group == 0, then we already have the data for it
      if(!block_group)
        return buffer;

      //the value of block_group to look up is non-existent
      if(block_group > (u32int)(buffer->total_blocks / buffer->blocks_per_group) - 1);
        return 0;

      //get the actual lba that the user want to read
      lba = block_group * buffer->blocks_per_group;
      floppy_read(lba, EXT2_BLOCK_SZ, (u32int*)buffer);

      return buffer;
    }
    case 1: //write to group descriptors
    {
      break;
    }
    case 2:
    {
      break;
    }
    case 3:
    {
      break;
    }
    case 4:
    {
      break;
    }
    default:
      return 0;
  }
}
