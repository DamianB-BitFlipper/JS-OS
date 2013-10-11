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

#define TO_U32INT(bytes)                   ((bytes) / sizeof(u32int)) 

#define BLOCKS_TO_SECTORS(blocks)          (((blocks) * EXT2_BLOCK_SZ) / SECTOR_SIZE)
#define SECTORS_TO_BLOCKS(sectors)         (((sectors) * SECTOR_SIZE) / EXT2_BLOCK_SZ)

#define IS_CONS_BLOCKS(first, second)      ((((second) - (first)) / (EXT2_BLOCK_SZ / SECTOR_SIZE)) == 1 ? TRUE : FALSE)

//the global path for the current directory
ext2_inode_t *ext2_root;
char *ext2_root_name;

//caches
ext2_superblock_t *ext2_g_sblock = 0;
ext2_group_descriptor_t *ext2_g_gdesc = 0;
ext2_inode_t *ext2_g_inode_table = 0;
u8int *ext2_g_bb = 0;                                   //the block bitmap
u8int *ext2_g_ib = 0;                                    //the inode bitmap
//caches

//defaults (logged as user) for permisions files need to have in order to be accesed
u32int _Rlogged = EXT2_I_RUSR, _Wlogged = EXT2_I_WUSR, _Xlogged = EXT2_I_XUSR;

static ext2_inode_t *__create_root__(void);
static ext2_inode_t *__create_file__(u32int size);
static ext2_inode_t *__create_dir__(ext2_superblock_t *sblock, ext2_group_descriptor_t *gdesc);
static char *__get_name_of_dir__(ext2_inode_t *directory);
static char *__get_name_of_file__(ext2_inode_t *directory, ext2_inode_t *file);

static struct ext2_dirent dirent;

//The start of the open file linked list
//~ volatile ext2_open_files_t *ext2_open_queue;

enum __block_types__
{
  EXT2_DIRECT,
  EXT2_SINGLY,
  EXT2_DOUBLY,
  EXT2_TRIPLY
};

u32int ext2_read(ext2_inode_t *node, u32int offset, u32int size, u8int *buffer)
{
  //if the user can read it
  if(node->mode & _Rlogged)
  {

    //a size of 0 or an offset greater than the node's size is impossible
    if(!size || offset > node->size)
      return 1; //fail

    //make sure we cap the size
    if(size + offset > node->size)
      size = node->size - offset;

    u32int blocks_to_read = ((size - 1) / EXT2_BLOCK_SZ) + 1, i, out;

    for(i = 0; i < blocks_to_read; i++)
    {
      out = ext2_block_of_set(node, i, (u32int*)(buffer + i * EXT2_BLOCK_SZ));

      if(!out)
        return 1; //fail
    }

    //sucess!
    return 0;
  }
}

u32int ext2_read_meta_data(ext2_superblock_t **sblock, ext2_group_descriptor_t **gdesc)
{
  ext2_superblock_t *sdata;
  
  //get the sblock data
  sdata = (ext2_superblock_t*)kmalloc(sizeof(ext2_superblock_t));
  floppy_read((u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE), sizeof(ext2_superblock_t), (u32int*)sdata);

  //check here if the superblock exists
  if(sdata->magic != EXT2_MAGIC)
    return 1; //fail!

  u32int nblockgroups = sdata->total_blocks / sdata->blocks_per_group;

  ext2_group_descriptor_t *gdata;

  //get the group descriptor data
  gdata = (ext2_group_descriptor_t*)kmalloc(nblockgroups * sizeof(ext2_group_descriptor_t));
  floppy_read((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / SECTOR_SIZE, nblockgroups * sizeof(ext2_group_descriptor_t), (u32int*)gdata);

  *sblock = sdata;
  //~ k_printf("junk location %h, actual %h\n", *sblock, sdata);

  *gdesc = gdata;

  //TODO make this work with the mutltiple sblocks and gdescs due to possible multiple block groups
  //~ memcpy(ext2_g_sblock, sdata, sizeof(ext2_superblock_t));
  //~ memcpy(ext2_g_gdesc, gdata, sizeof(ext2_group_descriptor_t));

  ext2_g_sblock = sdata;
  ext2_g_gdesc = gdata;

  //Sucess!
  return 0;
}

u32int *ext2_format_block_bitmap(ext2_group_descriptor_t *gdesc, u32int blocks_used)
{
  u32int location = gdesc->block_bitmap;

  //the offset from the beginning of the file (the end of the inode table)
  u32int begining_offset = gdesc->inode_table_id + BLOCKS_TO_SECTORS(gdesc->inode_table_size);

  u8int *block_bitmap;

  if(!ext2_g_bb)
  {
    block_bitmap = (u8int*)kmalloc(EXT2_BLOCK_SZ);
    ext2_g_bb = block_bitmap;
    floppy_read(location, EXT2_BLOCK_SZ, (u32int*)block_bitmap);
  }else{
    block_bitmap = ext2_g_bb;
  }

  u32int *output;//, *test;
  output = (u32int*)kmalloc(blocks_used * sizeof(u32int));

  memset(output, 0x0, blocks_used * sizeof(u32int));
  
  //the number of consecutive blocks free
  u32int consec_free = 0;
  s32int bit_off;

  //TODO this loop looks for consecutive blocks, if none are found implement non-consecutive block allocation
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

        //offset output to the end of the place where we will be assigning block locations
        output += blocks_used;

        //going backwards
        for(consec_free; consec_free > 0; consec_free--)
        {
          //flip the current bit and go backwards
          *(block_bitmap + off) |= bit_mask;

          output--;
          //assign the offset in bits from the beginning
          *output = BLOCKS_TO_SECTORS((8 * off + bit_off)) + begining_offset;

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

        //purposly not freeing since block_bitmap, regardless of update, is the global variable which should not be cleared

        //TODO in the case of non-consecutive blocks, make this optimized and work
        u32int i, *clear;
        clear = (u32int*)kmalloc(EXT2_BLOCK_SZ * blocks_used);

        memset(clear, 0x0, EXT2_BLOCK_SZ * blocks_used);

        //if we have exited, then all of the blocks are sequential, so write as one big chunk
        floppy_write(clear, EXT2_BLOCK_SZ * blocks_used, *output);

        kfree(clear);

        return output;
      }
    }

  }

  //purposly not freeing since block_bitmap, regardless of update, is the global variable which should not be cleared
  //~ kfree(block_bitmap);
  kfree(output);

  //if we did not exit yet, there must be no space
  return 0;
}

u32int ext2_singly_create(u32int *block_locations, u32int offset, u32int nblocks, ext2_group_descriptor_t *gdesc)
{
  u32int *location, *block_data, blk, value;
  block_data = (u32int*)kmalloc(EXT2_BLOCK_SZ);

  location = ext2_format_block_bitmap(gdesc, 1);
    
  //write the locations of the singly indirect blocks in the singly block in the memory (block_data)
  for(blk = 0; blk < (nblocks < EXT2_NIND_BLOCK ? nblocks : EXT2_NIND_BLOCK); blk++)
    *(block_data + blk) = *(block_locations + offset + blk);

  //write the singly block to the physical floppy disk
  floppy_write(block_data, EXT2_BLOCK_SZ, *location);

  //store the value of location in another place so we can free location and return its location
  value = *location;

  kfree(block_data);
  kfree(location);

  //return the location of the singly block
  return value;
  
}

u32int ext2_doubly_create(u32int *block_locaitions, u32int offset, u32int nblocks, ext2_group_descriptor_t *gdesc)
{
  u32int *location, *block_data, blk, value;
  block_data = (u32int*)kmalloc(EXT2_BLOCK_SZ);

  //clear the block data for the new block
  memset(block_data, 0x0, EXT2_BLOCK_SZ);

  //get a new location for the doubly block
  location = ext2_format_block_bitmap(gdesc, 1);

  //write the locations of the singly indirect blocks to the doubly block in the tmp memory block (block_data)
  for(blk = 0; blk < (nblocks < EXT2_NDIND_BLOCK ? nblocks : EXT2_NDIND_BLOCK); blk += EXT2_NIND_BLOCK)
  {
    *(block_data + (blk / EXT2_NIND_BLOCK)) = ext2_singly_create(block_locaitions, offset + blk,
                                                                 (blk + EXT2_NIND_BLOCK) <= nblocks ? EXT2_NIND_BLOCK : 
                                                                 nblocks % EXT2_NIND_BLOCK, gdesc);
  }

  floppy_write(block_data, EXT2_BLOCK_SZ, *location);

  value = *location;

  kfree(block_data);
  kfree(location);

  //return the location of the singly block
  return value;

}

u32int ext2_inode_entry_blocks(ext2_inode_t *node, ext2_group_descriptor_t *gdesc, u32int *block_locations, u32int blocks_used)
{
  u32int blk;

  //to begin, write all of the blocks a 0, so that there is no accidental junk
  for(blk = 0; blk < EXT2_N_BLOCKS; blk++)
    node->blocks[blk] = 0;

  //write the locations of the direct blocks
  for(blk = 0; blk < (blocks_used < EXT2_NDIR_BLOCKS ? blocks_used : EXT2_NDIR_BLOCKS); blk++)
    node->blocks[blk] = *(block_locations + blk);

  if(blocks_used < EXT2_NDIR_BLOCKS)
  {
    //we already wrote all of the data needed to be written above, return with sucess
    return 0;
  }else{
    u32int *location, *block_data;
    block_data = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    
    //clear the block data for the new block
    memset(block_data, 0x0, EXT2_BLOCK_SZ);

    //subtract the direct blocks that we have accounted for
    blocks_used -= EXT2_NDIR_BLOCKS;

    location = ext2_format_block_bitmap(gdesc, 1);
    
    //write the locations of the singly indirect blocks
    for(blk = 0; blk < (blocks_used < EXT2_NIND_BLOCK ? blocks_used : EXT2_NIND_BLOCK); blk++)
      *(block_data + blk) = *(block_locations + EXT2_NDIR_BLOCKS + blk);    

    floppy_write(block_data, EXT2_BLOCK_SZ, *location);
    
    //write the location of the singly block
    node->blocks[EXT2_NDIR_BLOCKS] = *location;

    //write the doubly blocks
    if(blocks_used > EXT2_NIND_BLOCK)
    {      
      //subtract the singly indirect blocks that we have accounted for
      blocks_used -= EXT2_NIND_BLOCK;

      //clear the block data for the new block
      memset(block_data, 0x0, EXT2_BLOCK_SZ);

      //get a new location for the doubly block
      location = ext2_format_block_bitmap(gdesc, 1);

      //write the locations of the doubly indirect blocks
      for(blk = 0; blk < (blocks_used < EXT2_NDIND_BLOCK ? blocks_used : EXT2_NDIND_BLOCK); blk += EXT2_NIND_BLOCK)
      {
        *(block_data + (blk / EXT2_NIND_BLOCK)) = ext2_singly_create(block_locations, EXT2_NDIR_BLOCKS + blk,
                                                                     (blk + EXT2_NIND_BLOCK) <= blocks_used ? EXT2_NIND_BLOCK : 
                                                                     blocks_used % EXT2_NIND_BLOCK, gdesc);
      }

      floppy_write(block_data, EXT2_BLOCK_SZ, *location);

      //write the location of the doubly block
      node->blocks[EXT2_NDIR_BLOCKS + 1] = *location;

      //write the triply blocks
      if(blocks_used > EXT2_NDIND_BLOCK)
      {
        //subtract the singly indirect blocks that we have accounted for
        blocks_used -= EXT2_NTIND_BLOCK;

        //clear the block data for the new block
        memset(block_data, 0x0, EXT2_BLOCK_SZ);

        //get a new location for the triply block
        location = ext2_format_block_bitmap(gdesc, 1);
  
        //write the locations of the triply indirect blocks
        for(blk = 0; blk < (blocks_used < EXT2_NTIND_BLOCK ? blocks_used : EXT2_NTIND_BLOCK); blk += EXT2_NDIND_BLOCK)
        {
          *(block_data + (blk / EXT2_NIND_BLOCK)) = ext2_doubly_create(block_locations, EXT2_NIND_BLOCK + EXT2_NDIR_BLOCKS + blk,
                                                                       (blk + EXT2_NDIND_BLOCK) <= blocks_used ? EXT2_NDIND_BLOCK : 
                                                                       blocks_used % EXT2_NDIND_BLOCK, gdesc);
        }

        floppy_write(block_data, EXT2_BLOCK_SZ, *location);

        //write the location of the triply block
        node->blocks[EXT2_NDIR_BLOCKS + 2] = *location;
      }
    }

    kfree(location);
    kfree(block_data);

  }
}

u32int ext2_data_to_inode_table(ext2_inode_t *data, ext2_group_descriptor_t *gdesc, ext2_superblock_t *sblock)
{
  ext2_inode_t *buffer;
  //~ u32int update;

  if(!ext2_g_inode_table)
  {
    buffer = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);
    ext2_g_inode_table = buffer;

    floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)buffer);

  }else
    buffer = ext2_g_inode_table;

  //loop until a free space has opened up
  u32int off = 0;
  while(buffer[off].nlinks && off < sblock->inodes_per_group)
    off++;

  //if we did not find enough space, return an error
  if(off == sblock->inodes_per_group)
    return 1; //error

  memcpy((u8int*)buffer + sizeof(ext2_inode_t) * off, data, sizeof(ext2_inode_t));
  
  //write the new inode table buffer
  floppy_write((u32int*)buffer, gdesc->inode_table_size * EXT2_BLOCK_SZ, gdesc->inode_table_id);
  
  //purposly not freeing since buffer, either way with update, is the global variable which should not be cleared

  //sucess!, return where we put it
  return off;
 
}

u32int ext2_inode_from_inode_table(u32int inode_number, ext2_inode_t *output, ext2_group_descriptor_t *gdesc)
{
  ext2_inode_t *buffer;

  if(!ext2_g_inode_table)
  {
    buffer = ext2_get_inode_table(gdesc);
    ext2_g_inode_table = buffer;
  }else
    buffer = ext2_g_inode_table;
  
  memcpy(output, (u8int*)buffer + sizeof(ext2_inode_t) * inode_number, sizeof(ext2_inode_t));

  //purposly not freeing since buffer, either way with update, is the global variable which should not be cleared

  //sucess!
  return 0;
}

ext2_inode_t *ext2_file_from_dir(ext2_inode_t *dir, char *name)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  ext2_inode_t *inode_table;

  if(!ext2_g_inode_table)
  {
    inode_table = ext2_get_inode_table(gdesc);
    ext2_g_inode_table = inode_table;
  }else
    inode_table = ext2_g_inode_table;

  ext2_inode_t *inode;
  inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

  if(dir->type == EXT2_DIR)
  {
    struct ext2_dirent *dirent2;

    u32int i;
    for(i = 0; i < sblock->total_inodes; i++)
    {
      //get the dirent information at index i
      dirent2 = ext2_dirent_from_dir(dir, i);

      if(dirent2)
      {
        //the input name matches to the dirent2.name that we got
        if(!strcmp(name, dirent2->name))
        {
          
          inode = ext2_inode_from_offset(dirent2->ino, inode_table);

          kfree(dirent2->name);
          //~ kfree(inode);
          kfree(sblock);
          kfree(gdesc);

          return inode;
        }
      }
      kfree(dirent2->name);
    }
  }

  kfree(inode);
  kfree(sblock);
  kfree(gdesc);
  //no file found, error
  return 0;
}

struct ext2_dirent *ext2_dirent_from_dir_data(ext2_inode_t *dir, u32int index, u32int *data)
{
  if(dir->type == EXT2_DIR) //just to check if the input node is a directory
  {

    u32int i = 0;
    u32int loop = 0, b = 0;

    //this no data has been passed
    if(!data)
      return 0;

    //loop forever, we will break when we find it or return and exit if we do not
    for(;;)
    {
      //if the loop equals the index we are looking for
      if(loop == index)
      {
        //if the rec_len of the direct has contents
        if(*(u16int*)((u8int*)data + i + sizeof(dirent.ino)))
        {

          static struct ext2_dirent dirent2;

          //extract the dirent information at the offset of i
          dirent2.ino = *(u32int*)((u8int*)data + i);
          dirent2.rec_len = *(u16int*)((u8int*)data + i + sizeof(dirent2.ino));
          dirent2.name_len = *(u8int*)((u8int*)data + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len));
          dirent2.file_type = *(u8int*)((u8int*)data + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len));

          //clears junk that may be contained in memory when kmallocing
          dirent2.name = (char*)kmalloc(dirent2.name_len + 1);

          //clears junk that may be contained in memory when kmallocing
          memset(dirent2.name, 0, dirent2.name_len + 1);

          //copies the name to dirent2.name
          memcpy(dirent2.name, (char*)((u8int*)data + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len) + sizeof(dirent2.file_type)), dirent2.name_len + 1);

          *(dirent2.name + dirent2.name_len) = 0; //Adds terminating 0 to string

          return &dirent2;

        }else{
          //error
          return 0;
        }

      }else{

        //this dirent is not the last one (there are more dirents after this one)
        if(*(u16int*)((u8int*)data + i + sizeof(dirent.ino)))
        {
          //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
          i += *(u16int*)((u8int*)data + i + sizeof(dirent.ino));
          loop++;
        }else //this is the last direct, if we have found nothing, exit
            return 0;

      }

    }

  }else{
    return 0;
  }

}

struct ext2_dirent *ext2_dirent_from_dir(ext2_inode_t *dir, u32int index)
{
  if(dir->type == EXT2_DIR) //just to check if the input node is a directory
  {

    u32int i = 0;
    u32int loop = 0, b = 0, *block;

    block = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    ext2_block_of_set(dir, b, block);

    //this dir has no blocks assigned
    if(!block)
    {
      kfree(block);
      return 0;
    }

    //loop forever, we will break when we find it or return and exit if we do not
    for(;;)
    {
      //if the loop equals the index we are looking for
      if(loop == index)
      {
        //if the rec_len of the direct has contents
        if(*(u16int*)((u8int*)block + i + sizeof(dirent.ino)))
        {

          static struct ext2_dirent dirent2;

          //extract the dirent information at the offset of i
          dirent2.ino = *(u32int*)((u8int*)block + i);
          dirent2.rec_len = *(u16int*)((u8int*)block + i + sizeof(dirent2.ino));
          dirent2.name_len = *(u8int*)((u8int*)block + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len));
          dirent2.file_type = *(u8int*)((u8int*)block + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len));

          //clears junk that may be contained in memory when kmallocing
          dirent2.name = (char*)kmalloc(dirent2.name_len + 1);

          //clears junk that may be contained in memory when kmallocing
          memset(dirent2.name, 0, dirent2.name_len + 1);

          //copies the name to dirent2.name
          memcpy(dirent2.name, (char*)((u8int*)block + i + sizeof(dirent2.ino) + sizeof(dirent2.rec_len) + sizeof(dirent2.name_len) + sizeof(dirent2.file_type)), dirent2.name_len + 1);

          *(dirent2.name + dirent2.name_len) = 0; //Adds terminating 0 to string

          kfree(block);
          return &dirent2;

        }else{

          kfree(block);
          //error
          return 0;
        }

      }else{

        //this dirent is not the last one (there are more dirents after this one)
        if(*(u16int*)((u8int*)block + i + sizeof(dirent.ino)))
        {
          //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
          i += *(u16int*)((u8int*)block + i + sizeof(dirent.ino));
          loop++;
        }else{ //this is the last direct, add 1 to block and reset the offset (i)
          i = 0;
          b++;

          //recalculate the block address
          ext2_block_of_set(dir, b, block);

          //this dir has not blocks assigned
          if(!block)
          {
            kfree(block);
            return 0;
          }
        }

      }

    }

  }else{
    return 0;
  }
  
}

u32int ext2_block_of_set(ext2_inode_t *file, u32int block_number, u32int *block_output)
{
  //if the block is a direct block
  if(block_number >= 0 && block_number < EXT2_NDIR_BLOCKS)
  {

    if(!file->blocks[block_number])
    {
      block_output = 0;
      return 0;
    }

    floppy_read(file->blocks[block_number], EXT2_BLOCK_SZ, block_output);

    return file->blocks[block_number];
    
  //if the block is in the singly set
  }else if(block_number >= EXT2_NDIR_BLOCKS && block_number < EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK)
  {
    //the offset to the block inside the singly
    u32int singly_offset = block_number - EXT2_NDIR_BLOCKS;

    u32int *singly;
    singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    if(!file->blocks[EXT2_IND_BLOCK])
    {
      kfree(singly);
      block_output = 0;
      return 0;
    }

    floppy_read(file->blocks[EXT2_IND_BLOCK], EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    if(!block)
    {
      kfree(singly);
      block_output = 0;
      return 0;
    }

    floppy_read(block, EXT2_BLOCK_SZ, block_output);

    kfree(singly);

    return block;

  //if the block is in the doubly set
  }else if(block_number >= EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK &&
          block_number < EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK)
  {
    u32int offset = block_number - (EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK);
    u32int doubly_offset = offset / EXT2_NIND_BLOCK;
    u32int singly_offset = offset % EXT2_NIND_BLOCK;

    u32int *doubly, *singly;
    singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    doubly = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    //error checking is always good
    if(!file->blocks[EXT2_DIND_BLOCK])
    {
      kfree(singly);
      kfree(doubly);
      block_output = 0;
      return 0;
    }

    floppy_read(file->blocks[EXT2_DIND_BLOCK], EXT2_BLOCK_SZ, doubly);

    u32int singly_block_location = *(doubly + doubly_offset);

    //error checking is always good
    if(!singly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      block_output = 0;
      return 0;
    }

    floppy_read(singly_block_location, EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    //error checking is always good
    if(!block)
    {
      kfree(singly);
      kfree(doubly);
      block_output = 0;
      return 0;
    }

    floppy_read(block, EXT2_BLOCK_SZ, block_output);

    //free the stuff allocated
    kfree(doubly);
    kfree(singly);

    return block;

  //if the block is in the triply set
  }else if(block_number >= EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK &&
          block_number < EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK + EXT2_NTIND_BLOCK)
  {
    u32int offset = block_number - (EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK);
    u32int triply_offset = offset / EXT2_NDIND_BLOCK;
    u32int doubly_offset = (offset % EXT2_NDIND_BLOCK) / EXT2_NIND_BLOCK;
    u32int singly_offset = (offset % EXT2_NDIND_BLOCK) % EXT2_NIND_BLOCK;

    u32int *triply, *doubly, *singly;
    triply = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    doubly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    if(!file->blocks[EXT2_TIND_BLOCK])
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      block_output = 0;
      return 0;
    }

    floppy_read(file->blocks[EXT2_TIND_BLOCK], EXT2_BLOCK_SZ, triply);

    u32int doubly_block_location = *(triply + triply_offset);

    if(!doubly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      block_output = 0;
      return 0;
    }

    floppy_read(doubly_block_location, EXT2_BLOCK_SZ, doubly);

    u32int singly_block_location = *(doubly + doubly_offset);

    if(!singly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      block_output = 0;
      return 0;
    }

    floppy_read(singly_block_location, EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    if(!block)
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      block_output = 0;
      return 0;
    }

    floppy_read(block, EXT2_BLOCK_SZ, block_output);

    kfree(triply);
    kfree(doubly);
    kfree(singly);

    return block;
  }else{

    block_output = 0;

    //the block number is not in range, error
    return 0;
  }

}

u32int ext2_write_block_of_set(ext2_inode_t *file, u32int block_number, u32int *block_data, u32int size)
{
  if(!size)
    return 0;
  else if(size > EXT2_BLOCK_SZ)
    size = EXT2_BLOCK_SZ;

  //if the block is a direct block
  if(block_number >= 0 && block_number < EXT2_NDIR_BLOCKS)
  {
    if(!file->blocks[block_number])
      return 0;

    floppy_write(block_data, size, file->blocks[block_number]);

    return file->blocks[block_number];
    
  //if the block is in the singly set
  }else if(block_number >= EXT2_NDIR_BLOCKS && block_number < EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK)
  {
    //the offset to the block inside the singly
    u32int singly_offset = block_number - EXT2_NDIR_BLOCKS;

    u32int *singly;
    singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    if(!file->blocks[EXT2_IND_BLOCK])
    {
      kfree(singly);
      return 0;
    }

    floppy_read(file->blocks[EXT2_IND_BLOCK], EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    if(!block)
    {
      kfree(singly);
      return 0;
    }

    floppy_write(block_data, size, block);

    kfree(singly);

    return block;

  //if the block is in the doubly set
  }else if(block_number >= EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK &&
          block_number < EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK)
  {
    u32int offset = block_number - EXT2_NDIR_BLOCKS;
    u32int doubly_offset = offset / EXT2_NIND_BLOCK;
    u32int singly_offset = offset % EXT2_NIND_BLOCK;

    u32int *doubly, *singly;
    singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    doubly = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    if(!file->blocks[EXT2_DIND_BLOCK])
    {
      kfree(singly);
      kfree(doubly);
      return 0;
    }

    floppy_read(file->blocks[EXT2_DIND_BLOCK], EXT2_BLOCK_SZ, doubly);

    u32int singly_block_location = *(doubly + doubly_offset);

    if(!singly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      return 0;
    }

    floppy_read(singly_block_location, EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    if(!block)
    {
      kfree(singly);
      kfree(doubly);
      return 0;
    }

    floppy_write(block_data, size, block);

    kfree(doubly);
    kfree(singly);

    return block;

  //if the block is in the triply set
  }else if(block_number >= EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK &&
          block_number < EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK + EXT2_NTIND_BLOCK)
  {
    u32int offset = block_number - EXT2_NDIR_BLOCKS;
    u32int triply_offset = offset / EXT2_NDIND_BLOCK;
    u32int doubly_offset = (offset % EXT2_NDIND_BLOCK) / EXT2_NIND_BLOCK;
    u32int singly_offset = (offset % EXT2_NDIND_BLOCK) % EXT2_NIND_BLOCK;

    u32int *triply, *doubly, *singly;
    triply = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    doubly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
    singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    if(!file->blocks[EXT2_TIND_BLOCK])
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      return 0;
    }

    floppy_read(file->blocks[EXT2_TIND_BLOCK], EXT2_BLOCK_SZ, triply);

    u32int doubly_block_location = *(triply + triply_offset);

    if(!doubly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      return 0;
    }

    floppy_read(doubly_block_location, EXT2_BLOCK_SZ, doubly);

    u32int singly_block_location = *(doubly + doubly_offset);

    if(!singly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      return 0;
    }

    floppy_read(singly_block_location, EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    if(!block)
    {
      kfree(singly);
      kfree(doubly);
      kfree(triply);
      return 0;
    }

    floppy_write(block_data, size, block);

    kfree(triply);
    kfree(doubly);
    kfree(singly);

    return block;
  }else
    return 0; //the block number is not in range, error
}

u32int ext2_add_file_to_dir(ext2_inode_t *parent_dir, ext2_inode_t *file, u32int file_type, char *filename)
{
  struct ext2_dirent dirent;

  u32int lengthOfName = strlen(filename);

  if(lengthOfName > 255)
    lengthOfName = 255;

  dirent.ino = file->inode;

  //The size of this whole struct basically
  dirent.rec_len = sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type) + lengthOfName + 1; //+1 is NULL terminating \000
  dirent.name_len = (u8int)lengthOfName;
  dirent.file_type = file_type;

  //~ k_printf("dirent.name is :%d\n", *dirent.name);

  //+1 being the \000 NULL termination 0 byte at the end of the string
  dirent.name = (char*)kmalloc(lengthOfName + 1);

  //copy the name of the file (fileNode->name) to the dirent.name
  memcpy(dirent.name, filename, lengthOfName); //fileNode->name already had a \000 termination zero
  *(dirent.name + lengthOfName) = 0; //Just in case add a \000 at the end

  u32int i, b, *block, location;

  //TODO make this search by chunk not block
  block = (u32int*)kmalloc(EXT2_BLOCK_SZ);

  /*In this section, we find a valid offset (i) and block number (b)
   * to an open dirent space
   *
   * length - 1 because if length == EXT2_BLOCK_SZ, there should be only one
   * block checked, but w/o that -1, 2 will be checked */
  for(b = 0; b <= (u32int)((parent_dir->size - 1) / EXT2_BLOCK_SZ); b++)
  {
    i = 0;
    location = ext2_block_of_set(parent_dir, b, block);

    if(!location)
      return 1; //error

    //this dir has not blocks assigned
    if(!block)
    {
      kfree(block);
      kfree(dirent.name);
      return 1; //error
    }

    //loop until we hit the end of the current block or get an open dirent
    while(*(u16int*)((u8int*)block + i + sizeof(dirent.ino)))
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i = i + *(u16int*)((u8int*)block + i + sizeof(dirent.ino));

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a block can hold, exit and go to new block
      if(i + dirent.rec_len >= EXT2_BLOCK_SZ)
        break;

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a direcotory can hold, exit function before page fault happens
      if(b * EXT2_BLOCK_SZ + i + dirent.rec_len >= parent_dir->size)
      {
        //expand parent_dir by one block
        if(ext2_expand(parent_dir, EXT2_BLOCK_SZ))
          return 1; //failed, out of directory left over space
      }

    }

    //if i is a valid offset, do not go to a new block, just exit
    if(!*(u16int*)((u8int*)block + i + sizeof(dirent.ino)))
      break;

  }

  //assigns the contents of the struct dirent to the directory contents location
  memcpy((u8int*)block + i, &dirent, dirent.rec_len - dirent.name_len);

  strcpy((u8int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type), dirent.name);

  floppy_write(block, EXT2_BLOCK_SZ, location);

  kfree(block);

  kfree(dirent.name);

  return 0;
}

u32int ext2_free_blocks(u32int *block_locs, u32int nblocks)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc; 

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  u8int *block_bitmap;

  if(!ext2_g_bb)
  {
    block_bitmap = (u8int*)kmalloc(EXT2_BLOCK_SZ);
    ext2_g_bb = block_bitmap;
    floppy_read(gdesc->block_bitmap, EXT2_BLOCK_SZ, (u32int*)block_bitmap);
  }else{
    block_bitmap = ext2_g_bb;
  }

  //the offset from the beginning of the file (the end of the inode table)
  //- the extra BLOCKS_TO_SECTORS(1), is to offset this to the begining of the last block
  u32int begining_offset = gdesc->inode_table_id + BLOCKS_TO_SECTORS(gdesc->inode_table_size) - 
    BLOCKS_TO_SECTORS(1);

  u32int byte = 0, bit = 0, i, error = 0, changes = FALSE;
  for(i = 0; i < nblocks; i++)
  {
    //if the block location is 0, then move on to the next block location value
    if(!*(block_locs + i))
      continue;
    
    byte = SECTORS_TO_BLOCKS(*(block_locs + i) - begining_offset) / 8;
    bit = 0b10000000 >> ((SECTORS_TO_BLOCKS(*(block_locs + i) - begining_offset) % 8) - 1);

    //if byte is too large, continue on and set to return an error at the end
    if(byte > EXT2_BLOCK_SZ)
    {
      error = 1;
      continue;
    }

    //just checking so we do not xor a 0 (off) bit and make it 1 (on) accidentally
    if(*(block_bitmap + byte) & bit)
    {
      *(block_bitmap + byte) ^= bit;
      if(changes == FALSE)
        changes = TRUE;
    }
  }

  if(changes == TRUE)
    floppy_write((u32int*)block_bitmap, EXT2_BLOCK_SZ, gdesc->block_bitmap);

  return error;
}

u32int ext2_expand(ext2_inode_t *node, u32int increase_bytes)
{
  //no point to expand, return sucess
  if(!increase_bytes)
    return 0; //sucess
  
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc; 

  //get the meta data if not already cached
  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  u32int *initial_locs, *all_locs, *added_locs, orig_nblocks, added_nblocks, all_nblocks;
  
  orig_nblocks = ((node->size - 1) / EXT2_BLOCK_SZ) + 1;
  added_nblocks = ((increase_bytes - 1) / EXT2_BLOCK_SZ) + 1;
  all_nblocks = orig_nblocks + added_nblocks;

  all_locs = (u32int*)kmalloc(sizeof(u32int) * all_nblocks);

  //retrieve the preexisting block locations
  initial_locs = ext2_block_locs(node);

  //retrieve the new block locations
  added_locs = ext2_format_block_bitmap(gdesc, added_nblocks);

  //concatonate all of the blocks into one array
  memcpy(all_locs, initial_locs, sizeof(u32int) * orig_nblocks);
  memcpy(all_locs + orig_nblocks, added_locs, sizeof(u32int) * added_nblocks);

  //remove the singly, doubly, and triply blocks from the block bitmap
  u32int *blocks_to_rm, i;
  blocks_to_rm = (u32int*)kmalloc(sizeof(u32int) * (EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS));
  
  for(i = 0; i < EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS; i++)
    *(blocks_to_rm + i) = *(node->blocks + EXT2_NDIR_BLOCKS + i);
 
  //when the for loop exits, i will equal the number of blocks that have been saved in blocks_to_rm
  if(ext2_free_blocks(blocks_to_rm, i))
  {

    kfree(initial_locs);
    kfree(added_locs);
    kfree(all_locs);
    kfree(blocks_to_rm);

    return 1; //there was some sort of error
  }

  //index the new blocks into the node's data
  ext2_inode_entry_blocks(node, gdesc, all_locs, all_nblocks);

  kfree(initial_locs);
  kfree(added_locs);
  kfree(all_locs);
  kfree(blocks_to_rm);

  //sucess!
  return 0;
}

u32int ext2_remove_inode_entry(ext2_inode_t *node)
{
  ext2_inode_t *buffer;
  u8int *ib;

  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc; 

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  if(!ext2_g_inode_table)
  {
    buffer = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);
    ext2_g_inode_table = buffer;

    floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)buffer);

  }else
    buffer = ext2_g_inode_table;

  if(!ext2_g_ib)
  {    
    ib = (u8int*)kmalloc(EXT2_BLOCK_SZ);
    ext2_g_ib = ib;

    floppy_read(gdesc->inode_bitmap, EXT2_BLOCK_SZ, (u32int*)ib);
  }else
    ib = ext2_g_ib;

  //loop until we found out inode entry
  u32int off = 0;
  while(buffer[off].inode != node->inode && off < sblock->inodes_per_group)
    off++;

  //we did not find out inode entry
  if(off == sblock->inodes_per_group)
    return 1; //error

  //flip the inode bitmap bit
  u32int byte, bit;
  byte = off / 8;
  bit = 0b10000000 >> ((off % 8) - 1);

  //check if the bit is 1, so we do not accidentally change a 0 to a 1
  if(*(ib + byte) & bit)
     *(ib + byte) ^= bit;
  else
    return 1; //there was no trace of the inode in the inode bitmap, error

  //clear the inode entry
  memset(&buffer[off], 0x0, sizeof(ext2_inode_t));

  //write our changes
  floppy_write((u32int*)buffer, gdesc->inode_table_size * EXT2_BLOCK_SZ, gdesc->inode_table_id);
  floppy_write((u32int*)ib, EXT2_BLOCK_SZ, gdesc->inode_bitmap);

  //sucess!
  return 0;
}

u32int ext2_delete(ext2_inode_t *parent_dir, ext2_inode_t *node)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc; 

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 1; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  //remove the inode entry from the inode table and the inode bitmap
  ext2_remove_inode_entry(node);

  //remove the singly, doubly, and triply block locations from the block bitmap, allong with the block_locs
  u32int *block_locs, nblocks, *blocks_to_rm, i;
  nblocks = ((node->size - 1) / EXT2_BLOCK_SZ) + 1;

  block_locs = ext2_block_locs(node);

  /*(EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS) are the singly, doubly, and triply block locations in the
   * inode structure, nblocks are the data blocks that must be freed, we compress these two block
   * block locations into one for efficientcy*/
  blocks_to_rm = (u32int*)kmalloc(sizeof(u32int) * ((EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS) + nblocks));
  
  for(i = 0; i < EXT2_N_BLOCKS - EXT2_NDIR_BLOCKS; i++)
    *(blocks_to_rm + i) = *(node->blocks + EXT2_NDIR_BLOCKS + i);
 
  memcpy(blocks_to_rm + i, block_locs, sizeof(u32int) * nblocks);

  //when the for loop exits, i + nblocks will equal the number of blocks that have been saved in blocks_to_rm
  if(ext2_free_blocks(blocks_to_rm, i + nblocks))
  {
    kfree(block_locs);
    kfree(blocks_to_rm);

    return 1; //there was some sort of error
  }

  //increment the number of free inodes and blocks freed
  gdesc->free_inodes++;
  gdesc->free_blocks += nblocks;

  floppy_write((u32int*)gdesc, sizeof(ext2_group_descriptor_t), gdesc->gdesc_location);

  //remove the file's dirent in the parent directory
  u32int b, *block, location;

  //TODO make this search by chunk not block
  block = (u32int*)kmalloc(EXT2_BLOCK_SZ);

  /*In this section, we find a valid offset (i) and block number (b)
   * to an open dirent space
   *
   * length - 1 because if length == EXT2_BLOCK_SZ, there should be only one
   * block checked, but w/o that -1, 2 will be checked */
  for(b = 0; b <= (u32int)((parent_dir->size - 1) / EXT2_BLOCK_SZ); b++)
  {
    //reset i
    i = 0;
    location = ext2_block_of_set(parent_dir, b, block);

    if(!location)
    {
      kfree(block_locs);
      kfree(blocks_to_rm);
      return 1; //error
    }
    
    //this dir has not blocks assigned
    if(!block)
    {

      kfree(block);
      kfree(block_locs);
      kfree(blocks_to_rm);
      return 1; //error
    }

    //loop until we hit the end of the current block or get an our dirent
    while(*(u32int*)((u8int*)block + i) != node->inode && 
          *(u8int*)((u8int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len)) != node->type)
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i += *(u16int*)((u8int*)block + i + sizeof(dirent.ino));

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a block can hold, exit and go to new block
      if(i + dirent.rec_len >= EXT2_BLOCK_SZ)
        break;

      /*if the offset (i) + the length of the contents in the struct dirent 
       * is greater than what a direcotory can hold, exit function, 
       * we did not find our file in this directory*/
      if(b * EXT2_BLOCK_SZ + i + dirent.rec_len >= parent_dir->size)
        return 1; //failed!

    }

    //if i is a valid offset, do not go to a new block, just exit
    if(*(u32int*)((u8int*)block + i) == node->inode && 
       *(u8int*)((u8int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len)) == node->type)
      break;

  }

  u32int rec_len = *(u16int*)((u8int*)block + i + sizeof(dirent.ino));

  /*shifts the dirent data in the directory over the one we want to delete, thus deleting its trails
   * the "-1 *" is used to show shift to the left */
  shiftData((u8int*)block + i + rec_len, -1 * rec_len, EXT2_BLOCK_SZ - i - rec_len);

  //write the changed block
  floppy_write(block, EXT2_BLOCK_SZ, location);

  kfree(block);
  kfree(block_locs);
  kfree(blocks_to_rm);

  //sucess!
  return 0;
}

ext2_inode_t *ext2_create_dir(ext2_inode_t *parent_dir, char *name)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc; 

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  ext2_inode_t *dir;
  dir = __create_dir__(sblock, gdesc);

  /*initrd_root (sblock->first_inode) is the "mother root" directory, a directory
   * that contains root and only root directory, if the parentNode is
   * initrd_root, then we are creating root, and root has its own special
   * hard links that are made outside of this function, else, make those hard
   * links */
  if(parent_dir->inode != sblock->first_inode)
  {
    ext2_add_file_to_dir(parent_dir, dir, dir->type, name);

    ext2_add_hardlink_to_dir(dir, dir, "."); //adds hardlink for the current directory
    ext2_add_hardlink_to_dir(dir, parent_dir, ".."); //adds hardlink for the parent directory

  }

  //purposly not freeing since buffer, either way with update, is the global variable which should not be cleared

  return dir;
}

static ext2_inode_t *__create_dir__(ext2_superblock_t *sblock, ext2_group_descriptor_t *gdesc)
{
  //a new directory will consist of one block for the '.' and '..' directories
  u32int blocks_used = 1, *block_locations;

  block_locations = ext2_format_block_bitmap(gdesc, blocks_used);
  
  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

  data->magic = M_EXT2;
  data->inode = ext2_find_open_inode(sblock, gdesc);
  data->mode = EXT2_I_RUSR | EXT2_I_WUSR | EXT2_I_XUSR | EXT2_I_RGRP | EXT2_I_XGRP | EXT2_I_ROTH | EXT2_I_XOTH;
  data->type = EXT2_DIR;
  data->uid = 0;
  data->size = blocks_used * EXT2_BLOCK_SZ;
  data->atime = posix_time();
  data->ctime = posix_time();
  data->mtime = posix_time();
  data->dtime = 0;
  data->gid = 0;
  data->nlinks = 2;    //number of hard links (itself and the '.' directory)
  data->nblocks = BLOCKS_TO_SECTORS(blocks_used);   //blocks of 512 bytes
  data->flags = 0;
  data->osd1 = EXT2_OS_JSOS;

  //add the block information
  ext2_inode_entry_blocks(data, gdesc, block_locations, blocks_used);
  
  data->version = 0;
  data->fire_acl = 0;
  data->dir_acl = 0;
  data->fragment_addr = 0;

  //add the inode data to the table
  ext2_data_to_inode_table(data, gdesc, sblock);

  //decrement the number of free inodes and blocks there are
  gdesc->free_inodes--;
  gdesc->free_blocks -= blocks_used;

  floppy_write((u32int*)gdesc, sizeof(ext2_group_descriptor_t), gdesc->gdesc_location);

  kfree(block_locations);
  
  return data;
}

u32int ext2_find_open_inode(ext2_superblock_t *sblock, ext2_group_descriptor_t *gdesc)
{
  u8int *ib;

  if(!ext2_g_ib)
  {    
    ib = (u8int*)kmalloc(EXT2_BLOCK_SZ);
    ext2_g_ib = ib;

    floppy_read(gdesc->inode_bitmap, EXT2_BLOCK_SZ, (u32int*)ib);
  }else
    ib = ext2_g_ib;

  //start inode at 1 since 0 is automatically taken by the mother root
  u32int bit = 0b10000000, byte = 0, inode = 1;

  while((*(ib + byte) & bit) && inode < sblock->total_inodes)
  {
    inode++;

    bit >>= 1;

    //if we have reached the end of bit, reset it and increment byte
    if(!bit)
    {
      bit = 0b10000000;
     byte++;
    }
  }

  //we are out of inodes
  if(inode == sblock->total_inodes)
  {
    //we have found nothing
    return -1;
  }
   
  *(ib + byte) |= bit;

  floppy_write((u32int*)ib, EXT2_BLOCK_SZ, gdesc->inode_bitmap);

  //purposly not freeing since sblock and gdesc, regardless of update, is the global variable which should not be cleared

  //after we exit the while loop, 'inode' is the open inode
  return inode;

}

ext2_inode_t *ext2_create_file(ext2_inode_t *parent_dir, char *name, u32int size)
{
  ext2_inode_t *file;
  file = __create_file__(size);

  ext2_add_file_to_dir(parent_dir, file, file->type, name);

  return file;
}

static ext2_inode_t *__create_file__(u32int size)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;
   
  //the the number of blocks the initial size will take up
  u32int blocks_used = (u32int)((size - 1) / EXT2_BLOCK_SZ) + 1;

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  u32int *block_locations;
  block_locations = ext2_format_block_bitmap(gdesc, blocks_used);

  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
  
  data->magic = M_EXT2;
  data->inode = ext2_find_open_inode(sblock, gdesc);
  data->mode = EXT2_I_RUSR | EXT2_I_WUSR | EXT2_I_RGRP | EXT2_I_WGRP | EXT2_I_ROTH | EXT2_I_WOTH;
  data->type = EXT2_FILE;
  data->uid = 0;
  data->size = size;
  data->atime = posix_time();
  data->ctime = posix_time();
  data->mtime = posix_time();
  data->dtime = 0;
  data->gid = 0;
  data->nlinks = 1;    //number of hard links, itself
  data->nblocks = BLOCKS_TO_SECTORS(blocks_used);   //blocks of 512 bytes
  data->flags = 0;
  data->osd1 = EXT2_OS_JSOS;
  
  ext2_inode_entry_blocks(data, gdesc, block_locations, blocks_used);
  
  data->version = 0;
  data->fire_acl = 0;
  data->dir_acl = 0;
  data->fragment_addr = 0;
  
  //add the inode data to the table
  ext2_data_to_inode_table(data, gdesc, sblock);
  
  //decrement the number of free inodes and blocks there are
  gdesc->free_inodes--;
  gdesc->free_blocks -= blocks_used;
  
  floppy_write((u32int*)gdesc, sizeof(ext2_group_descriptor_t), gdesc->gdesc_location);

  //purposly not freeing since sblock and gdesc, regardless of update, is the global variable which should not be cleared    

  kfree(block_locations);

  return data;
}

u32int ext2_write(ext2_inode_t *node, u32int offset, u32int size, u8int *buffer)
{
  //if the user can write to it
  if(node->mode & _Wlogged)
  {

    //a size of 0 or an offset greater than the node's size is impossible
    if(!size || offset > node->size)
      return 1; //fail

    //make sure we cap the size
    if(size + offset > node->size)
      size = node->size - offset;

    u32int blocks_to_write = ((size - 1) / EXT2_BLOCK_SZ) + 1, i, out;

    for(i = 0; i < blocks_to_write; i++)
    {
      out = ext2_write_block_of_set(node, i, (u32int*)(buffer + i * EXT2_BLOCK_SZ), size);

      if(!out)
        return 1; //fail
    }

    //sucess!
    return 0;
  }
}

u32int *ext2_get_singly(u32int location, u32int *nblocks)
{
  u32int i = 0, *singly, *locs;

  singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
  floppy_read(location, EXT2_BLOCK_SZ, singly);

  //after this while exits, i will equal the number of real blocks there are
  while(*(singly + i) && i < EXT2_NIND_BLOCK)
    i++;
     
  locs = (u32int*)kmalloc(sizeof(u32int) * i);

  //copies only the actual blocks, and not excessive 0's
  memcpy(locs, singly, i);

  *nblocks = i;

  kfree(singly);

  return locs;
}

u32int *ext2_get_doubly(u32int location, u32int *nblocks)
{
  u32int i = 0, a = 0, *doubly, *max_locs, *locs, *tmp_nblocks, total_blocks = 0;

  doubly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
  floppy_read(location, EXT2_BLOCK_SZ, doubly);

  //after this while exits, i will equal the number of real blocks there are in the doubly block
  while(*(doubly + i) && i < EXT2_NIND_BLOCK) //we use EXT2_NIND_BLOCK, since there are 256 singly block locations in a doubly block
    i++;
     
  //the maximum amout of blocks that the doubly block has
  max_locs = (u32int*)kmalloc(EXT2_NIND_BLOCK * sizeof(u32int) * i);

  tmp_nblocks = (u32int*)kmalloc(sizeof(u32int));

  for(a = 0; a < i; a++)
  {
    locs = ext2_get_singly(*(doubly + a), tmp_nblocks);
    memcpy(max_locs + a * EXT2_NIND_BLOCK, locs, *tmp_nblocks);
    total_blocks += *tmp_nblocks;
  }

  kfree(locs);
  //(a - 1) equals the number of full singly blocks there were, the last tmp_nblocks contains the last singly's size
  locs = (u32int*)kmalloc((a - 1) * EXT2_NIND_BLOCK + *tmp_nblocks);

  //copies only the actual blocks, and not excessive 0's
  memcpy(locs, max_locs, (a - 1) * EXT2_NIND_BLOCK + *tmp_nblocks);

  *nblocks = total_blocks;

  kfree(tmp_nblocks);
  kfree(max_locs);
  kfree(doubly);

  return locs;
}

u32int *ext2_get_triply(u32int location, u32int *nblocks)
{
  u32int i = 0, a = 0, *triply, *max_locs, *locs, *tmp_nblocks, total_blocks = 0;

  triply = (u32int*)kmalloc(EXT2_BLOCK_SZ);
  floppy_read(location, EXT2_BLOCK_SZ, triply);

  //after this while exits, i will equal the number of real blocks there are in the doubly block
  while(*(triply + i) && i < EXT2_NIND_BLOCK) //we use EXT2_NIND_BLOCK, since there are 256 doubly block locations in a triply block
    i++;
     
  //the maximum amout of blocks that the triply block has
  max_locs = (u32int*)kmalloc(EXT2_NDIND_BLOCK * sizeof(u32int) * i);

  tmp_nblocks = (u32int*)kmalloc(sizeof(u32int));

  for(a = 0; a < i; a++)
  {
    locs = ext2_get_doubly(*(triply + a), tmp_nblocks);
    memcpy(max_locs + a * EXT2_NDIND_BLOCK, locs, *tmp_nblocks);
    total_blocks += *tmp_nblocks;
  }

  kfree(locs);
  //(a - 1) equals the number of full doubly blocks there were, the last tmp_nblocks contains the last doubly's size
  locs = (u32int*)kmalloc((a - 1) * EXT2_NDIND_BLOCK + *tmp_nblocks);

  //copies only the actual blocks, and not excessive 0's
  memcpy(locs, max_locs, (a - 1) * EXT2_NDIND_BLOCK + *tmp_nblocks);

  *nblocks = total_blocks;

  kfree(tmp_nblocks);
  kfree(max_locs);
  kfree(triply);

  return locs;
}

u32int *ext2_block_locs(ext2_inode_t *node)
{
  if(!node || !node->size)
    return 0; //error

  u32int *locs;
  locs = (u32int*)kmalloc((((node->size - 1) / EXT2_BLOCK_SZ) + 1) * sizeof(u32int));

  enum __block_types__ __block_types__;

  //checks what is the max type of block the file has, used for optimizations
  if(!*(node->blocks + EXT2_IND_BLOCK) && !*(node->blocks + EXT2_DIND_BLOCK) && !*(node->blocks + EXT2_TIND_BLOCK))
    __block_types__ = EXT2_DIRECT;
  if(*(node->blocks + EXT2_IND_BLOCK) && !*(node->blocks + EXT2_DIND_BLOCK) && !*(node->blocks + EXT2_TIND_BLOCK))
    __block_types__ = EXT2_SINGLY;
  if(*(node->blocks + EXT2_IND_BLOCK) && *(node->blocks + EXT2_DIND_BLOCK) && !*(node->blocks + EXT2_TIND_BLOCK))
    __block_types__ = EXT2_DOUBLY;
  if(*(node->blocks + EXT2_IND_BLOCK) && *(node->blocks + EXT2_DIND_BLOCK) && *(node->blocks + EXT2_TIND_BLOCK))
    __block_types__ = EXT2_TRIPLY;

  switch(__block_types__)
  {
    case EXT2_DIRECT:
    {
      u32int i = 0;
      
      //copy the direct blocks until they end
      while(*(node->blocks + i))
      {
        *(locs + i) = *(node->blocks + i);
        i++;
      }
      //exit
      break;
    }
    case EXT2_SINGLY:
    {
      u32int i, *singly, *nblocks;

      //copy all of the direct blocks
      for(i = 0; i < EXT2_NDIR_BLOCKS; i++)
        *(locs + i) = *(node->blocks + i);

      nblocks = (u32int*)kmalloc(sizeof(u32int*));

      singly = ext2_get_singly(*(node->blocks + EXT2_NDIR_BLOCKS), nblocks);

      //reset i and then copy all of the singly blocks until they end
      memcpy(locs + EXT2_NDIR_BLOCKS, singly, *nblocks), 
      kfree(nblocks);
      kfree(singly);

      //exit
      break;
    }
    case EXT2_DOUBLY:
    {
      u32int i, *singly, *doubly, *nblocks;

      nblocks = (u32int*)kmalloc(sizeof(u32int*));

      //copy all of the direct blocks
      for(i = 0; i < EXT2_NDIR_BLOCKS; i++)
        *(locs + i) = *(node->blocks + i);

      singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
      floppy_read(*(node->blocks + EXT2_IND_BLOCK), EXT2_BLOCK_SZ, singly);

      //reset i and then copy all of the singly blocks until they end
      for(i = 0; i < EXT2_NIND_BLOCK; i++)
        *(locs + EXT2_NDIR_BLOCKS + i) = *(singly + i);        

      doubly = ext2_get_doubly(*(node->blocks + EXT2_DIND_BLOCK), nblocks);
      memcpy(locs + EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK, doubly, *nblocks);

      kfree(singly);
      kfree(doubly);
      kfree(nblocks);
      //exit
      break;
    }
    case EXT2_TRIPLY:
    {
      u32int i, *singly, *doubly, *triply, *nblocks;

      nblocks = (u32int*)kmalloc(sizeof(u32int*));

      //copy all of the direct blocks
      for(i = 0; i < EXT2_NDIR_BLOCKS; i++)
        *(locs + i) = *(node->blocks + i);

      singly = (u32int*)kmalloc(EXT2_BLOCK_SZ);
      floppy_read(*(node->blocks + EXT2_IND_BLOCK), EXT2_BLOCK_SZ, singly);

      //reset i and then copy all of the singly blocks until they end
      for(i = 0; i < EXT2_NIND_BLOCK; i++)
        *(locs + EXT2_NDIR_BLOCKS + i) = *(singly + i);        

      doubly = ext2_get_doubly(*(node->blocks + EXT2_DIND_BLOCK), nblocks);
      memcpy(locs + EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK, doubly, *nblocks);

      triply = ext2_get_triply(*(node->blocks + EXT2_TIND_BLOCK), nblocks);
      memcpy(locs + EXT2_NDIR_BLOCKS + EXT2_NIND_BLOCK + EXT2_NDIND_BLOCK, triply, *nblocks);

      kfree(singly);
      kfree(doubly);
      kfree(triply);
      kfree(nblocks);
      //exit
      break;
    }
    default:
    {
      //error
      return 0;
    }
  }

  return locs;
}

u32int *ext2_chunk_data(u32int *blocks, u32int nblocks, u32int chunk, u32int *out_chunk_size)
{

  //set the output chunk size to default
  *out_chunk_size = 0;

  //chunck cannot be greater or equal to the number of blocks there are (chunk starts from 0)
  if(chunk >= nblocks)
    return 0;

  //if there are no blocks
  if(!nblocks)
    return 0; //error

  //if there is only one block
  if(nblocks == 1)
  {
    u32int *data;
    data = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    floppy_read(*blocks, EXT2_BLOCK_SZ, data);

    *out_chunk_size = EXT2_BLOCK_SZ;
    return data;
  }

  //chunk_size is guaranteed to be atleast one block
  u32int chunks = 0, chunk_size = EXT2_BLOCK_SZ, first, second, i = 0, exit = FALSE, start, *data;

  //if we want to access the first chunk, then, we automatically want to exit at the beginning of the next chunk
  if(!chunk)
    exit = TRUE;

  first = *blocks;
  start = first;
  second = *(blocks + 1);

  //until we run out of blocks
  while(i < nblocks)
  {
    if(IS_CONS_BLOCKS(first, second) == FALSE)
    {
      //if we want the read the first chunk and the two blocks are not consecutive, just simply read from the beginning to first
      if(!chunk)
      {
        data = (u32int*)kmalloc(chunk_size);

        floppy_read(*blocks, chunk_size, data);

        *out_chunk_size = chunk_size;
        return data;
      }else{ //increment the first and second blocks, and the number of chunks, record where this chunk started
        chunks++;
        
        //if exit was to be TRUE, we need the location of our chunk, so do not override it
        if(exit == FALSE)
           start = second;

        i++;

        //there is no more and we did not find out chunk
        if((i + 1) == nblocks && chunks != chunk)
          return 0;
        else if((i + 1) == nblocks && chunks == chunk) //we found our chunk at the very end
        {
          //'second' is our chunk
          data = (u32int*)kmalloc(EXT2_BLOCK_SZ);

          floppy_read(second, EXT2_BLOCK_SZ, data);

          *out_chunk_size = EXT2_BLOCK_SZ;
          return data;
        }else{
          first = *(blocks + i);
          second = *(blocks + i + 1);
        }

        /*exit will only get set if we have entered our wanted block, 
         * once the program gets to this if statement, it is at the beginning
         * of the next chunk, now we have the appropriot size for our chunk and 
         * its initial location*/
        if(exit == TRUE)
        {
          data = (u32int*)kmalloc(chunk_size);

          floppy_read(start, chunk_size, data);

          *out_chunk_size = chunk_size;
          return data;
        }
      }

    }else{ //increment the first and second blocks
      i++;
      chunk_size += EXT2_BLOCK_SZ;

      //there is no more and we did not find out chunk
      if((i + 1) == nblocks && chunks != chunk)
        return 0;
      else if((i + 1) == nblocks && chunks == chunk) //we found our chunk at the very end
      {
        //read the data of the chunk
        data = (u32int*)kmalloc(chunk_size);

        floppy_read(start, chunk_size, data);

        *out_chunk_size = chunk_size;
        return data;
      }else{
        first = *(blocks + i);
        second = *(blocks + i + 1);
      }

    }

    /*once the chunk we are at (chunks) and the chunk we want to return (chunk),
     * we know that atleast one block must be in chunk_size, so set it to EXT2_BLOCK_SZ,
     * set that we want to exit*/
    if(chunks == chunk && exit == FALSE)
    {
      chunk_size = EXT2_BLOCK_SZ;

      exit = TRUE;
    }
  }

  //as a fail safe, if we exited the while loop that is an error.
  return 0;

}

u32int *ext2_open(ext2_inode_t *node, char *mask)
{
  u32int permission = 0;

  //set the permissions based on the mask
  permission = __open_fs_mask_to_u32int__(mask);

  FILE *open_file, *prev;
  open_file = (FILE*)kmalloc(sizeof(FILE));

  open_file->permisions = permission;
  open_file->fs_type = M_EXT2;
  open_file->node = (void*)node;
  open_file->next = 0;
  
  //place the typedef 'open_file' at the very end of the list
  prev = (FILE*)initial_fdesc;
  while(prev->next)
    prev = prev->next;

  prev->next = open_file;

  return 0; //sucess!
}

u32int ext2_close(ext2_inode_t *node)
{
  FILE *open_file, *prev;
  
  open_file = initial_fdesc;

  //finds the open node in the open files list
  while(open_file->node != node && open_file)
  {
    prev = open_file;
    open_file = open_file->next;
  }
  //the node must not be open, we did not find it
  if(!open_file)
    return 1; //error

  //remove the entry from the list
  prev->next = open_file->next;

  kfree(open_file);

  return 0; //sucess!
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

  data->reserved = 0;
  
  return;
  
}

u32int ext2_set_block_group(u32int size)
{
  //save the original size of the partition
  u32int orig_size = size;

  //we make u32int size equal the block size in bytes
  size = size > (EXT2_MAX_BLK_GRP_SZ * EXT2_BLOCK_SZ) ? (EXT2_MAX_BLK_GRP_SZ * EXT2_BLOCK_SZ) : size;
  u32int blocks_per_group = (u32int)(size / EXT2_BLOCK_SZ);
  u32int inodes_per_group = size == (EXT2_MAX_BLK_GRP_SZ * EXT2_BLOCK_SZ) ? EXT2_I_PER_GRP : EXT2_I_PER_GRP_FLPPY;
  u32int inode_table_size = (inodes_per_group * sizeof(ext2_inode_t)) / EXT2_BLOCK_SZ;
  u32int nblock_groups = (u32int)(orig_size / size);

  //super block data
  ext2_superblock_t *sblock_data;
  sblock_data = (ext2_superblock_t*)kmalloc(sizeof(ext2_superblock_t));

  if(!ext2_g_sblock)
    ext2_g_sblock = (ext2_superblock_t*)kmalloc(nblock_groups * sizeof(ext2_superblock_t));

  //group descriptor data
  ext2_group_descriptor_t *gdesc_table_data;
  //one group desciptor data
  gdesc_table_data = (ext2_group_descriptor_t*)kmalloc(sizeof(ext2_group_descriptor_t));

  //the buffer that will hold all of the group descriptors
  u8int *gdesc_buf;
  gdesc_buf = (u8int*)kmalloc(nblock_groups * EXT2_BLOCK_SZ);

  //for the gdesc cache
  if(!ext2_g_gdesc)
    ext2_g_gdesc = (ext2_group_descriptor_t*)kmalloc(nblock_groups * (nblock_groups * EXT2_BLOCK_SZ));
  
  //block bitmap, inode bitmap, inode table locations
  u32int block_BB, block_IB, block_IT;
  u8int write_data = 0x0;

  //block bitmap and inode bitmap data
  u8int *mem;
  mem = (u8int*)kmalloc(2 * EXT2_BLOCK_SZ); //two blocks for the block bitmap and inode bitmap combined
  memset(mem, write_data, 2 * EXT2_BLOCK_SZ);

  if(!ext2_g_bb)
    ext2_g_bb = (u8int*)kmalloc(nblock_groups * EXT2_BLOCK_SZ);

  if(!ext2_g_ib)
    ext2_g_ib = (u8int*)kmalloc(nblock_groups * EXT2_BLOCK_SZ);

  //inode table data
  u8int *i_tables;
  i_tables = (u8int*)kmalloc(sizeof(ext2_inode_t) * inodes_per_group);
  memset(i_tables, write_data, sizeof(ext2_inode_t) * inodes_per_group);

  if(!ext2_g_inode_table)
    ext2_g_inode_table = (ext2_inode_t*)kmalloc(nblock_groups * (sizeof(ext2_inode_t) * inodes_per_group));

  u32int nbgroup, group_offset;
  for(nbgroup = 0; nbgroup < nblock_groups; nbgroup++)
  {
    //group offset changes so the floppy_writes write to group 1, 2, 3... as nbgroup increments
    group_offset = nbgroup * blocks_per_group;
    
    //!write the primary super block to the floppy (SB)
    ext2_sblock_set_data(sblock_data, 0,
                          group_offset + (u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE),
                          EXT2_ERRORS_CONTINUE, 0, "Primary");
    sblock_data->total_inodes = nblock_groups * inodes_per_group;
    sblock_data->total_blocks = (u32int)(orig_size / EXT2_BLOCK_SZ);
    sblock_data->blocks_per_group = (u32int)(blocks_per_group);
    sblock_data->inodes_per_group = inodes_per_group;
    sblock_data->block_group_number = nbgroup; //the block group number that this superblock is located in

    memcpy(ext2_g_sblock + sizeof(ext2_superblock_t) * nbgroup, sblock_data, sizeof(ext2_superblock_t));

    floppy_write((u32int*)sblock_data, sizeof(ext2_superblock_t),
                  group_offset + (u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE)); //write to sector 2

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
    block_BB = group_offset + (u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE);
    block_IB = group_offset + (u32int)((EXT2_SBLOCK_OFF + 2 * EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE);

    //~ k_printf("BB %d IB %d\n", block_BB, block_IB);
    memcpy(ext2_g_bb + EXT2_BLOCK_SZ * nbgroup, mem, EXT2_BLOCK_SZ);
    memcpy(ext2_g_ib + EXT2_BLOCK_SZ * nbgroup, mem, EXT2_BLOCK_SZ);

    floppy_write((u32int*)mem, 2 * EXT2_BLOCK_SZ, block_BB); //write to sector 3 and 5
  
    //!write zeros over the inode tables (IT)
    block_IT = group_offset + (u32int)((EXT2_SBLOCK_OFF + 3 * EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE);
    
    memcpy(ext2_g_inode_table + sizeof(ext2_inode_t) * inodes_per_group * nbgroup, i_tables, sizeof(ext2_inode_t) * inodes_per_group);
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
      gdesc_table_data->inode_table_size = inode_table_size;

      gdesc_table_data->gdesc_location = group_offset + (u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / SECTOR_SIZE);

      //write the actual contents in the beggining of the EXT2_BLOCK_SZ of the buffer
      memcpy((u32int*)(gdesc_buf + gdesc * EXT2_BLOCK_SZ), (u32int*)gdesc_table_data, sizeof(ext2_group_descriptor_t));

      //fill the rest of the EXT2_BLOCK_SZ of the buffer that was not used with zeros
      memset((u32int*)(gdesc_buf + gdesc * EXT2_BLOCK_SZ + sizeof(ext2_group_descriptor_t)), 0x0, EXT2_BLOCK_SZ - sizeof(ext2_group_descriptor_t));
      
    }

    memcpy(ext2_g_gdesc + nbgroup * nblock_groups * EXT2_BLOCK_SZ, gdesc_buf, nblock_groups * EXT2_BLOCK_SZ);

    floppy_write((u32int*)gdesc_buf, nblock_groups * EXT2_BLOCK_SZ,
                group_offset + (u32int)((EXT2_SBLOCK_OFF + EXT2_BLOCK_SZ) / SECTOR_SIZE)); //write starting from sector 2
  }
  
  kfree(mem);
  kfree(i_tables);

  kfree(sblock_data);
  kfree(gdesc_table_data);
  kfree(gdesc_buf);

  //sucess!
  return 0;
}

ext2_inode_t *ext2_inode_from_offset(u32int inode_number, ext2_inode_t *inode_table)
{
  ext2_inode_t *buffer;
  buffer = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

  memcpy(buffer, (u8int*)inode_table + sizeof(ext2_inode_t) * inode_number, sizeof(ext2_inode_t));

  //sucess!
  return buffer;
}

ext2_inode_t *ext2_get_inode_table(ext2_group_descriptor_t *gdesc)
{
  ext2_inode_t *buffer;

  if(!ext2_g_inode_table)
  {
    buffer = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);
    floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)buffer);
    ext2_g_inode_table = buffer;
  }else
    buffer = ext2_g_inode_table;

  return buffer;
}

char *ext2_name_of_node(ext2_inode_t *parent_dir, ext2_inode_t *node)
{
  switch(node->type)
  {
  case EXT2_FILE:
    return __get_name_of_file__(parent_dir, node);
  case EXT2_DIR:
    return __get_name_of_dir__(node);
  default:
    return 0;
  }
}

static char *__get_name_of_file__(ext2_inode_t *directory, ext2_inode_t *file) 
{
  u32int i = 0;

  struct ext2_dirent *dirent;
  
  //loop forever untill we exit by sucess of an error
  for(i;;i++)
  {
    dirent = ext2_dirent_from_dir(directory, i);

    if(!dirent)
      return 0; //fail

    //have we found the name we are looking for
    if(dirent->ino == file->inode)
      return dirent->name;
  }

}

static char *__get_name_of_dir__(ext2_inode_t *directory) 
{

  if(directory->inode == ext2_root->inode)
    return ext2_root_name;

  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  ext2_inode_t *inode_table;
  inode_table = ext2_get_inode_table(gdesc);

  struct ext2_dirent *dirent;
  //get the inode of the parent, always the second index (1) starting from 0
  dirent = ext2_dirent_from_dir(directory, 1);

  ext2_inode_t *parent;
  parent = ext2_inode_from_offset(dirent->ino, inode_table);

  u32int i = 0, b = 0, *block;
  block = (u32int*)kmalloc(EXT2_BLOCK_SZ);
  if(!ext2_block_of_set(parent, b, block))
  {
    kfree(parent);
    kfree(block);
    return 0;
  }

  do
  {
    dirent = ext2_dirent_from_dir_data(parent, i, block);
    i++;

    if(!dirent)
    {
      b++;
      if(!ext2_block_of_set(parent, b, block))
      {
        kfree(parent);
        kfree(block);
        return 0;
      }
    }
  }
  while(dirent->file_type != EXT2_DIR || dirent->ino != directory->inode);

  char *name;
  name = dirent->name;
  //~ name = (char*)kmalloc(dirent->name_len + 1);
  //~ *(name + dirent->name_len) = 0;

  kfree(block);
  kfree(parent);

  return name;
}

u32int ext2_set_current_dir(ext2_inode_t *directory)
{
  kfree(path); //frees the contents of the char array pointer, path
  
  ptr_currentDir = directory; //sets the value of the dir to the ptr_cuurentDir

  //the root's inode is always 1, after the mother root's inode of 0
  if(directory->inode == 1)
  {
    //keep it simple, if root is the only directory, copy its name manually
    path = (char*)kmalloc(2); //2 chars beign "/" for root and \000
  
    *(path) = '/';
    *(path + 1) = 0; //added \000 to the end
    
    //sucess!
    return 0;
  }

  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 1; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;
  }

  ext2_inode_t *node;
  node = directory;
  ext2_inode_t *copy;
  
  u32int count = 0, totalCharLen = 0, allocated_names = 15, *name_locs;

  name_locs = (u32int*)kmalloc(allocated_names * sizeof(u32int));

  char *name;

  /*starts from the current directory, goes backwards by getting the node
   * of the parent (looking up the data in ".." dir), adding its namelen
   * to the u32int totalCharLen and getting its parent, etc.
   *
   *once the parent is the same as the child, that only occurs with the
   * root directory, so we should exit */
  do
  {
    copy = node;

    name = __get_name_of_dir__(copy);

    if(copy)
    {
      //+1 being the preceding "/" to every dir that will be added later
      totalCharLen = totalCharLen + strlen(name) + 1; 
      *(name_locs + count) = *(u32int*)&name;
      count++;
    }

    if(count == allocated_names)
    {
      allocated_names *= 2;

      u32int *tmp;
      tmp = (u32int*)kmalloc(allocated_names);
      memcpy(tmp, name_locs, allocated_names / 2);
      
      kfree(name_locs);
      name_locs = tmp;
    }

    node = ext2_file_from_dir(copy, "..");
  }
  while(node->inode != copy->inode);
  
  /*we have the root dir
   * that does not need a preceding "/" because we will get "//"
   * which is ugly and not right. Also the "/" before the very first
   * directory should not be there because it will look ugly with
   * the root "/". So the total totalCharLen should be
   * -2 to count for both of those instances*/
  totalCharLen -= 2;
  
  path = (char*)kmalloc(totalCharLen + 1); //+1 is for the \000 NULL terminating 0
  //~ strcpy(path, "/");
  
  //reset the copy back to the top (current directory)
  copy = directory;
  
  u32int i, charsWritten = 0, nameLen;
  for(i = 0; i < count; i++)
  {
    nameLen = strlen(*(name_locs + i));
  
    /* i < count - 2 is a protection from drawing the preceding "/"
     * on the first two dirs in the path (root and one more). The first two dirs will
     * allways be drawn the last two times (we write the dir names to path from
     * current dir (top) to root (bottom)), thus if i is less
     * than the count - 2, that means we are not yet at the last
     * two drawing and it is ok to have a precedding "/" */
    if(copy->inode != ext2_root->inode && i < count - 2)
    {
      memcpy(path + (totalCharLen - charsWritten - nameLen - 1), "/", 1);
      memcpy(path + (totalCharLen - charsWritten - nameLen), *(name_locs + i), nameLen);
      charsWritten = charsWritten + nameLen + 1; //increment charsWritten with the "/<name>" string we just wrote, +1 is that "/"
    }else{
      memcpy(path + (totalCharLen - charsWritten - nameLen), *(name_locs + i), nameLen);
      charsWritten = charsWritten + nameLen; //increment charsWritten with the "/" string we just wrote
    }
  
    //find the parent of copy
    node = ext2_file_from_dir(copy, "..");
    copy = node;
  }
  
  *(path + totalCharLen) = 0; //added \000 to the end of path
  
  //~ k_printf("\n\nPATH is: %s\n", path);
  
  kfree(name_locs);

  //sucess!
  return 0;
}

static ext2_inode_t *__create_root__()
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;

  if(!ext2_g_sblock || !ext2_g_gdesc)
  {
    if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
      return 0; //error
  }else{
    sblock = ext2_g_sblock;
    gdesc = ext2_g_gdesc;

  }
  
  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

  data->magic = M_EXT2;
  //the root is always with an inode of 0, the root inode
  data->inode = 0;

  data->mode = EXT2_I_RUSR | EXT2_I_WUSR | EXT2_I_XUSR | EXT2_I_RGRP | EXT2_I_XGRP | EXT2_I_ROTH | EXT2_I_XOTH;
  data->type = EXT2_DIR;
  data->uid = 0;
  data->size = 0;
  data->atime = posix_time();
  data->ctime = posix_time();
  data->mtime = posix_time();
  data->dtime = 0;
  data->gid = 0;
  data->nlinks = 2;    //number of hard links (itself and the '.' directory)
  data->nblocks = 0;   //blocks of 512 bytes
  data->flags = 0;
  data->osd1 = EXT2_OS_JSOS;

  //set all of the blocks to 0 (empty)
  u32int blank;
  for(blank = 0; blank < EXT2_N_BLOCKS; blank++)
    data->blocks[blank] = 0;
  
  data->version = 0;
  data->fire_acl = 0;
  data->dir_acl = 0;
  data->fragment_addr = 0;

  //add the inode data to the table
  ext2_data_to_inode_table(data, gdesc, sblock);

  //decrement the number of free inodes and blocks there are
  gdesc->free_inodes--;

  floppy_write((u32int*)gdesc, sizeof(ext2_group_descriptor_t), gdesc->gdesc_location);

  //purposly not freeing since sblock and gdesc, regardless of update, is the global variable which should not be cleared

  return data;
}

u32int ext2_initialize(u32int size)
{

  if(size > FLOPPY_SIZE)
    return 1;

  //if there is no persistent storage available
  if(_FloppyStorage == FALSE)
    return 2;

  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;
  
  /*if ext2_read_meta_data returns with anything but a 0,
   * then there is either an error in the sblock, or one does not exist*/
  if(ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc))
  {
    k_printf("The floppy disk does not have preexisting data on it\n");

    k_printf("\n\tCreating required ext2 structures, this may take some time\n");

    k_printf("\tCreating the superblock and group descriptor table...");
    ext2_set_block_group(size);
    k_printf("%cgdone%cw\n");

    ext2_inode_t *root; //, *file, *dir;

    k_printf("\tCreating the root directory...");
    //create the root directory
    root = ext2_create_dir(__create_root__(), "/");
    k_printf("%cgdone%cw\n");

    ext2_root = root;

    if(!ext2_root)
      return 1; //error

    path = (char*)kmalloc(2);
    *(path) = '/';
    *(path + 1) = 0;

    k_printf("\tCreating root \".\" hardlink...");
    if(ext2_add_hardlink_to_dir(root, root, ".")) //adds hardlink to root
      return 1; //error
    k_printf("%cgdone%cw\n");

    k_printf("\tCreating root \"..\" hardlink...");
    if(ext2_add_hardlink_to_dir(root, root, "..")) //adds hardlink to root
      return 1; //error
    k_printf("%cgdone%cw\n");

    //set the name of the root directory
    ext2_root_name = (char*)kmalloc(2);
    *(ext2_root_name) = '/';
    *(ext2_root_name + 1) = 0;

    ext2_set_current_dir(root); 

    //sucess!
    return 0;
  }else{
    k_printf("The floppy disk has preexisting data on it!\n");

    ext2_inode_t *inode_table;

    if(!ext2_g_inode_table)
    {
      inode_table = ext2_get_inode_table(gdesc);
      ext2_g_inode_table = inode_table;
    }else
      inode_table = ext2_g_inode_table;

    k_printf("\tAcquiring preexisting root directory...");
    //get the root, it is always second after the 'mother root'
    ext2_root = ext2_inode_from_offset(1, inode_table);
    k_printf("%cgdone%cw\n");

    if(!ext2_root)
      return 1; //error

    //set the name of the root directory
    ext2_root_name = (char*)kmalloc(2);
    *(ext2_root_name) = '/';
    *(ext2_root_name + 1) = 0;

    ext2_set_current_dir(ext2_root); 

    //sucess!
    return 0;
  }
}
