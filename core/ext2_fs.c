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

#define TO_U32INT(bytes)                   (bytes / sizeof(u32int)) 

#define BLOCKS_TO_SECTORS(blocks)          ((blocks * EXT2_BLOCK_SZ) / SECTOR_SIZE)

//the global path for the current directory
char *ext2_path;
u32int ext2_current_dir_inode = 0;
ext2_inode_t *ext2_root;

//defaults (logged as user) for permisions files need to have in order to be accesed
u32int _Rlogged = EXT2_I_RUSR, _Wlogged = EXT2_I_WUSR, _Xlogged = EXT2_I_XUSR;

static ext2_inode_t *__create_root__(void);
static ext2_inode_t *__create_file__(u32int size);
static ext2_inode_t *__create_dir__(ext2_superblock_t *sblock, ext2_group_descriptor_t *gdesc);
static char *__get_name_of_dir__(ext2_inode_t *directory, ext2_inode_t *inode_table);

static struct ext2_dirent dirent;

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

u32int *ext2_format_block_bitmap(ext2_group_descriptor_t *gdesc, u32int blocks_used)
{
  u32int location = gdesc->block_bitmap;

  //the offset from the beginning of the file (the end of the inode table)
  u32int begining_offset = gdesc->inode_table_id + BLOCKS_TO_SECTORS(gdesc->inode_table_size);

  u8int *block_bitmap;
  block_bitmap = (u8int*)kmalloc(EXT2_BLOCK_SZ);
  floppy_read(location, EXT2_BLOCK_SZ, (u32int*)block_bitmap);
    
  u32int *output;//, *test;
  output = (u32int*)kmalloc(blocks_used * sizeof(u32int));
  //~ test = (u32int*)kmalloc(8);

  //~ if(blocks_used == 2)
  //~ {
    //~ kfree(output);
    //~ k_printf("out is GOOOOOOONE\n");
    //~ kfree(test);
    //~ k_printf("I'M a FREEEEEE super freee\n");
    //~ for(;;);
  //~ }

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

        kfree(block_bitmap);

        return output;
      }
    }

  }

  kfree(block_bitmap);
  kfree(output);

  //if we did not exit yet, there must be no space
  return 0;
}

u32int ext2_singly_create(u32int *block_locations, u32int offset, u32int nblocks, ext2_group_descriptor_t *gdesc)
{
  u32int *location, *block_data, blk, value;
  block_data = (u32int*)kmalloc(EXT2_BLOCK_SZ);

  location = ext2_format_block_bitmap(gdesc, 1);
    
  //write the locations of the singly indirect blocks
  for(blk = 0; blk < (nblocks < EXT2_NIND_BLOCK ? nblocks : EXT2_NIND_BLOCK); blk++)
    *(block_data + blk) = *(block_locations + offset + blk);

  //write the singly block
  floppy_write(block_data, EXT2_BLOCK_SZ, *location);

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

  //write the locations of the singly indirect blocks to the doubly block
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

u32int ext2_inode_entry_blocks(ext2_inode_t *inode, ext2_group_descriptor_t *gdesc, u32int *block_locations, u32int blocks_used)
{
  u32int blk;

  //to begin, write all of the blocks a 0, so that there is no accidental junk
  for(blk = 0; blk < EXT2_N_BLOCKS; blk++)
    inode->blocks[blk] = 0;

  //write the locations of the direct blocks
  for(blk = 0; blk < (blocks_used < EXT2_NDIR_BLOCKS ? blocks_used : EXT2_NDIR_BLOCKS); blk++)
    inode->blocks[blk] = *(block_locations + blk);

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
    inode->blocks[EXT2_NDIR_BLOCKS] = *location;

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
      inode->blocks[EXT2_NDIR_BLOCKS + 1] = *location;

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
        inode->blocks[EXT2_NDIR_BLOCKS + 2] = *location;
      }
    }

    kfree(location);
    kfree(block_data);

  }
}

u32int ext2_data_to_inode_table(ext2_inode_t *data, ext2_group_descriptor_t *gdesc, ext2_superblock_t *sblock)
{
  ext2_inode_t *buffer;
  buffer = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);

  floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)buffer);

  //loop until a free space has opened up
  u32int off = 0;
  while(buffer[off].nlinks && off < sblock->inodes_per_group)
  {
    off++;
  }

  //if we did not find enough space, return an error
  if(off == sblock->inodes_per_group)
  {
    kfree(buffer);

    //error
    return 1;
  }

  memcpy((u8int*)buffer + sizeof(ext2_inode_t) * off, data, sizeof(ext2_inode_t));
  
  //write the new inode table buffer
  floppy_write((u32int*)buffer, gdesc->inode_table_size * EXT2_BLOCK_SZ, gdesc->inode_table_id);
  
  kfree(buffer);

  //sucess!, return where we put it
  return off;
 
}

u32int ext2_inode_from_inode_table(u32int inode_number, ext2_inode_t *output, ext2_group_descriptor_t *gdesc)
{
  ext2_inode_t *buffer;
  buffer = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);

  floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)buffer);

  memcpy(output, (u8int*)buffer + sizeof(ext2_inode_t) * inode_number, sizeof(ext2_inode_t));

  kfree(buffer);

  //sucess!
  return 0;
}

ext2_inode_t *ext2_file_from_dir(ext2_inode_t *dir, char *name)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;

  ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);

  ext2_inode_t *inode;
  inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t*));

  //~ //if the directory's inode is 0, it is the main root, and only the '/' root should exist in it
  //~ if(!dir->inode)
  //~ {
    //~ ext2_inode_from_inode_table(dir->inode, inode, gdesc);    
    //~ return inode;
  //~ }

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
          kfree(dirent2->name);
          kfree(inode);
          kfree(sblock);
          kfree(gdesc);

          //~ return &root_nodes[dirent2->ino];
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

struct ext2_dirent *ext2_dirent_from_dir(ext2_inode_t *dir, u32int index)
{
  if(dir->type == EXT2_DIR) //just to check if the input node is a directory
  {

    u32int i = 0;
    u32int loop = 0, b = 0, *block;

    block = (u32int*)kmalloc(EXT2_BLOCK_SZ);

    ext2_block_of_set(dir, b, block);

    //this dir has not blocks assigned
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
      block_output = 0;
      return 0;
    }

    floppy_read(file->blocks[EXT2_DIND_BLOCK], EXT2_BLOCK_SZ, doubly);

    u32int singly_block_location = *(doubly + doubly_offset);

    if(!singly_block_location)
    {
      kfree(singly);
      kfree(doubly);
      block_output = 0;
      return 0;
    }

    floppy_read(singly_block_location, EXT2_BLOCK_SZ, singly);

    u32int block = *(singly + singly_offset);

    if(!block)
    {
      kfree(singly);
      kfree(doubly);
      block_output = 0;
      return 0;
    }

    floppy_read(block, EXT2_BLOCK_SZ, block_output);

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

  k_printf("dirent.name is :%d\n", *dirent.name);

  //+1 being the \000 NULL termination 0 byte at the end of the string
  dirent.name = (char*)kmalloc(lengthOfName + 1);

  //copy the name of the file (fileNode->name) to the dirent.name
  memcpy(dirent.name, filename, lengthOfName); //fileNode->name already had a \000 termination zero
  *(dirent.name + lengthOfName) = 0; //Just in case add a \000 at the end

  u32int i, b, *block, location;

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

    //this dir has not blocks assigned
    if(!block)
    {
      kfree(block);
      kfree(dirent.name);
      return 0;
    }

    //loop until we hit the end of the current block or get an open dirent
    while(*(u16int*)((u8int*)block + i + sizeof(dirent.ino)))
    {
      //increase i with the rec_len that we get by moving fileheader sizeof(dirent.ino) (4 bytes) and reading its value
      i = i + *(u16int*)((u8int*)block + i + sizeof(dirent.ino));

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a block can hold, exit and go to new block
      if(i + dirent.rec_len >= EXT2_BLOCK_SZ)
      {
        break;
      }

      //if the offset (i) + the length of the contents in the struct dirent is greater than what a direcotory can hold, exit function before page fault happens
      if(b * EXT2_BLOCK_SZ + i + dirent.rec_len >= parent_dir->size)
      {
        //As of now, return with an error, but eventually expand the file size
        //TODO make expand file function

        //failed, out of directory left over space
        return 1;
      }

    }

    //if i is a valid offset, do not go to a new block, just exit
    if(!*(u16int*)((u8int*)block + i + sizeof(dirent.ino)))
    {
      break;
    }

  }

  //assigns the contents of the struct dirent to the directory contents location
  memcpy((u8int*)block + i, &dirent, dirent.rec_len - dirent.name_len);

  strcpy((u8int*)block + i + sizeof(dirent.ino) + sizeof(dirent.rec_len) + sizeof(dirent.name_len) + sizeof(dirent.file_type), dirent.name);

  floppy_write(block, EXT2_BLOCK_SZ, location);

  kfree(block);

  k_printf("PLEASE DONT SHOWWWWW!!!!\n");

  //TODO freeing this causes assertion
  kfree(dirent.name);

  k_printf("FREEE==========================================================\n");

}

ext2_inode_t *ext2_create_dir(ext2_inode_t *parent_dir, char *name)
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc; 

  ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);

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
    ext2_add_hardlink_to_dir(parent_dir, dir, ".."); //adds hardlink for the parent directory

  }

  // free all of the data
  kfree(sblock);
  kfree(gdesc);

  return dir;
}

static ext2_inode_t *__create_dir__(ext2_superblock_t *sblock, ext2_group_descriptor_t *gdesc)
{
  //a new directory will consist of one block for the '.' and '..' directories
  u32int blocks_used = 1, *block_locations;

  block_locations = ext2_format_block_bitmap(gdesc, blocks_used);
  
  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

  data->inode = ext2_find_open_inode(gdesc);
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

  u32int *test;
  test = (u32int*)kmalloc(EXT2_BLOCK_SZ);
  floppy_read(60, EXT2_BLOCK_SZ, test);


  kfree(test);
  kfree(block_locations);
  
  return data;
}

u32int ext2_find_open_inode(ext2_group_descriptor_t *gdesc)
{
  u32int i = 0;

  u32int inodes_per_block = EXT2_BLOCK_SZ / sizeof(ext2_inode_t);

  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);

  floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)data);

  while(data[i].inode || data[i].nlinks)
  {
    i++;

    //we are out of inodes
    if(i == gdesc->inode_table_size * inodes_per_block)
    {
      //we have found nothing
      return -1;
    }
  }

  //after we exit the while loop, 'i' is the open inode
  return i;

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
  
  ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);
  
  u32int *block_locations;
  block_locations = ext2_format_block_bitmap(gdesc, blocks_used);

  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
  
  data->inode = ext2_find_open_inode(gdesc);
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
  
  k_printf("INODE ENTRY BLOCKS\n");

  ext2_inode_entry_blocks(data, gdesc, block_locations, blocks_used);
  
  data->version = 0;
  data->fire_acl = 0;
  data->dir_acl = 0;
  data->fragment_addr = 0;
  
  k_printf("INODE data to table\n");

  //add the inode data to the table
  ext2_data_to_inode_table(data, gdesc, sblock);
  
  //decrement the number of free inodes and blocks there are
  gdesc->free_inodes--;
  gdesc->free_blocks -= blocks_used;
  
  floppy_write((u32int*)gdesc, sizeof(ext2_group_descriptor_t), gdesc->gdesc_location);
  
  k_printf("file to dir\n");

  //add the file to the parent directory
  //~ ext2_add_file_to_dir(parentNode, data, name);
  
  // free all of the data
  kfree(sblock);
  kfree(gdesc);

  k_printf("IMMMMM TRAIYIIKFDSJALKFSLJ trying");
  kfree(block_locations);
  k_printf("ROGER: and out\n");

  return data;
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
                          group_offset + (u32int)((EXT2_SBLOCK_OFF) / SECTOR_SIZE),
                          EXT2_ERRORS_CONTINUE, 0, "Primary");
    sblock_data->total_inodes = nblock_groups * inodes_per_group;
    sblock_data->total_blocks = (u32int)(orig_size / EXT2_BLOCK_SZ);
    sblock_data->blocks_per_group = (u32int)(blocks_per_group);
    sblock_data->inodes_per_group = inodes_per_group;
    sblock_data->block_group_number = nbgroup; //the block group number that this superblock is located in
    
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

    k_printf("BB %d IB %d\n", block_BB, block_IB);
  
    floppy_write((u32int*)mem, 2 * EXT2_BLOCK_SZ, block_BB); //write to sector 3 and 5
  
    //!write zeros over the inode tables (IT)
    block_IT = group_offset + (u32int)((EXT2_SBLOCK_OFF + 3 * EXT2_BLOCK_SZ + EXT2_BLOCK_SZ * nblock_groups) / SECTOR_SIZE);
    
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
      memcpy((u32int*)(gdesc_buf + gdesc * SECTOR_SIZE), (u32int*)gdesc_table_data, sizeof(ext2_group_descriptor_t));

      //fill the rest of the EXT2_BLOCK_SZ of the buffer that was not used with zeros
      memset((u32int*)(gdesc_buf + gdesc * SECTOR_SIZE + sizeof(ext2_group_descriptor_t)), 0x0, EXT2_BLOCK_SZ - sizeof(ext2_group_descriptor_t));
      
    }

    floppy_write((u32int*)gdesc_buf, nblock_groups * SECTOR_SIZE,
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

/* static char *__get_name_of_dir__(ext2_inode_t *directory)
 * {
 *   ext2_superblock_t *sblock;  
 *   ext2_group_descriptor_t *gdesc;
 *   
 *   ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);
 *   
 *   struct ext2_dirent *dirent;
 *   //get the inode of the parent, always the second index (1) starting from 0
 *   dirent = ext2_dirent_from_dir(directory, 1);
 * 
 *   ext2_inode_t *parent;
 *   parent = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
 *   ext2_inode_from_inode_table(dirent->ino, parent, gdesc);
 * 
 *   u32int i = 0;
 *   do
 *   {
 *     dirent = ext2_dirent_from_dir(parent, i);
 *   }
 *   while(dirent->file_type != EXT2_DIR || dirent->ino != directory->inode);
 * 
 *   //save the name to a different buffer so we can free the dirent
 *   char *name;
 *   name = (char*)kmalloc(dirent->name_len + 1); //+1 for the \000
 *   memcpy(name, dirent->name, dirent->name_len);
 * 
 *   *(name + dirent->name_len) = 0;
 * 
 *   kfree(sblock);
 *   kfree(gdesc);
 *   kfree(parent);
 *   kfree(dirent);
 * 
 *   return name;
 * }
 * 
 * //TODO make set current directory work
 * u32int ext2_set_current_dir(ext2_inode_t *directory)
 * {
 *   kfree(ext2_path); //frees the contents of the char array pointer, ext2_path
 *   
 *   currentDir_inode = directory->inode; //sets the value of the dir inode to the cuurentDir_inode
 *   
 *   ext2_inode_t *node = directory;
 *   ext2_inode_t *copy;
 *   
 *   u32int count = 0, totalCharLen = 0;
 *   char *name;
 * 
 *   /\*starts from the current directory, goes backwards by getting the node
 *    * of the parent (looking up the data in ".." dir), adding its namelen
 *    * to the u32int totalCharLen and getting its parent, etc.
 *    *
 *    *once the parent is the same as the child, that only occurs with the
 *    * root directory, so we should exit *\/
 *   do
 *   {
 *     //TODO make this more efficient
 *     copy = node;
 *     name = __get_name_of_dir__(copy);
 *     if(copy)
 *     {
 *       //+1 being the preceding "/" to every dir that will be added later
 *       totalCharLen = totalCharLen + strlen(name) + 1; 
 *       count++;
 *     }
 *   
 *     node = finddir_fs(copy, "..");
 *     kfree(name);
 *   }
 *   while(node != copy);
 *   
 *   if(count > 1) //if we are in a directory other than root
 *   {
 *     /\*we have the root dir
 *      * that does not need a preceding "/" because we will get "//"
 *      * which is ugly and not right. Also the "/" before the very first
 *      * directory should not be there because it will look ugly with
 *      * the root "/". So the total totalCharLen should be
 *      * -2 to count for both of those instances*\/
 *     totalCharLen -= 2;
 *   
 *     ext2_path = (char*)kmalloc(totalCharLen + 1); //+1 is for the \000 NULL terminating 0
 *     //~ strcpy(ext2_path, "/");
 *   
 *     //reset the copy back to the top (current directory)
 *     copy = directory;
 *   
 *     u32int i, charsWritten = 0, nameLen;
 *     for(i = 0; i < count; i++)
 *     {
 *       nameLen = strlen(copy->name);
 *   
 *       /\* i < count - 2 is a protection from drawing the preceding "/"
 *        * on the first two dirs in the ext2_path (root and one more). The first two dirs will
 *        * allways be drawn the last two times (we write the dir names to ext2_path from
 *        * current dir (top) to root (bottom)), thus if i is less
 *        * than the count - 2, that means we are not yet at the last
 *        * two drawing and it is ok to have a precedding "/" *\/
 *       if(copy != fs_root && i < count - 2)
 *       {
 *         memcpy(ext2_path + (totalCharLen - charsWritten - nameLen - 1), "/", 1);
 *         memcpy(ext2_path + (totalCharLen - charsWritten - nameLen), copy->name, nameLen);
 *         charsWritten = charsWritten + nameLen + 1; //increment charsWritten with the "/<name>" string we just wrote, +1 is that "/"
 *   
 *       }else{
 *         memcpy(ext2_path + (totalCharLen - charsWritten - nameLen), copy->name, nameLen);
 *         charsWritten = charsWritten + nameLen; //increment charsWritten with the "/" string we just wrote
 *   
 *       }
 *   
 *       //find the parent of copy
 *       node = finddir_fs(copy, "..");
 *       copy = node;
 *     }
 *   
 *     *(ext2_path + totalCharLen) = 0; //added \000 to the end of ext2_path
 *   
 *   }else{
 *     //keep it simple, if root is the only directory, copy its name manually
 *     ext2_path = (char*)kmalloc(2); //2 chars beign "/" for root and \000
 *   
 *     *(ext2_path) = '/';
 *     *(ext2_path + 1) = 0; //added \000 to the end
 *   }
 *   
 *   k_printf("\n\nEXT2_PATH is: %s\n", ext2_path);
 *   
 *   //sucess!
 *   return 0;
 * } */

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
  buffer = (ext2_inode_t*)kmalloc(gdesc->inode_table_size * EXT2_BLOCK_SZ);

  floppy_read(gdesc->inode_table_id, gdesc->inode_table_size * EXT2_BLOCK_SZ, (u32int*)buffer);

  return buffer;
}

static char *__get_name_of_dir__(ext2_inode_t *directory, ext2_inode_t *inode_table) 
{
  struct ext2_dirent *dirent;
  //get the inode of the parent, always the second index (1) starting from 0
  dirent = ext2_dirent_from_dir(directory, 1);

  ext2_inode_t *parent;
  parent = ext2_inode_from_offset(dirent->ino, inode_table);

  u32int i = 0;
  do
  {
    dirent = ext2_dirent_from_dir(parent, i);
    i++;
  }
  while(dirent->file_type != EXT2_DIR || dirent->ino != directory->inode);

  char *name;
  name = dirent->name;
  //~ name = (char*)kmalloc(dirent->name_len + 1);
  //~ *(name + dirent->name_len) = 0;

  kfree(parent);
  kfree(dirent);

  return name;
}

//TODO make set current directory work
u32int ext2_set_current_dir(ext2_inode_t *directory)
{
  kfree(ext2_path); //frees the contents of the char array pointer, ext2_path
  
  ext2_current_dir_inode = directory->inode; //sets the value of the dir inode to the cuurentDir_inode

  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;
  
  ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);

  ext2_inode_t *inode_table;
  inode_table = ext2_get_inode_table(gdesc);
  
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
    name = __get_name_of_dir__(directory, inode_table);

    if(copy)
    {
      //+1 being the preceding "/" to every dir that will be added later
      totalCharLen = totalCharLen + strlen(name) + 1; 
      count++;
    }

    *(name_locs + count) = *(u32int*)&name;

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
  while(node != copy);
  
  if(count > 1) //if we are in a directory other than root
  {
    /*we have the root dir
     * that does not need a preceding "/" because we will get "//"
     * which is ugly and not right. Also the "/" before the very first
     * directory should not be there because it will look ugly with
     * the root "/". So the total totalCharLen should be
     * -2 to count for both of those instances*/
    totalCharLen -= 2;
  
    ext2_path = (char*)kmalloc(totalCharLen + 1); //+1 is for the \000 NULL terminating 0
    //~ strcpy(ext2_path, "/");
  
    //reset the copy back to the top (current directory)
    copy = directory;
  
    u32int i, charsWritten = 0, nameLen;
    for(i = 0; i < count; i++)
    {
      nameLen = strlen(*(name_locs + i));
  
      /* i < count - 2 is a protection from drawing the preceding "/"
       * on the first two dirs in the ext2_path (root and one more). The first two dirs will
       * allways be drawn the last two times (we write the dir names to ext2_path from
       * current dir (top) to root (bottom)), thus if i is less
       * than the count - 2, that means we are not yet at the last
       * two drawing and it is ok to have a precedding "/" */
      if(copy->inode != ext2_root->inode && i < count - 2)
      {
        memcpy(ext2_path + (totalCharLen - charsWritten - nameLen - 1), "/", 1);
        memcpy(ext2_path + (totalCharLen - charsWritten - nameLen), *(name_locs + i), nameLen);
        charsWritten = charsWritten + nameLen + 1; //increment charsWritten with the "/<name>" string we just wrote, +1 is that "/"
      }else{
        memcpy(ext2_path + (totalCharLen - charsWritten - nameLen), *(name_locs + i), nameLen);
        charsWritten = charsWritten + nameLen; //increment charsWritten with the "/" string we just wrote
      }
  
      //find the parent of copy
      node = ext2_file_from_dir(copy, "..");
      copy = node;
    }
  
    *(ext2_path + totalCharLen) = 0; //added \000 to the end of ext2_path
  
  }else{
    //keep it simple, if root is the only directory, copy its name manually
    ext2_path = (char*)kmalloc(2); //2 chars beign "/" for root and \000
  
    *(ext2_path) = '/';
    *(ext2_path + 1) = 0; //added \000 to the end
  }
  
  k_printf("\n\nEXT2_PATH is: %s\n", ext2_path);
  
  kfree(name_locs);

  //sucess!
  return 0;
}

static ext2_inode_t *__create_root__()
{
  ext2_superblock_t *sblock;  
  ext2_group_descriptor_t *gdesc;
  
  ext2_read_meta_data((ext2_superblock_t**)&sblock, (ext2_group_descriptor_t**)&gdesc);
  
  ext2_inode_t *data;
  data = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));

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

  // free all of the data
  kfree(sblock);
  kfree(gdesc);

  return data;
}

u32int ext2_initialize(u32int size)
{
  ext2_set_block_group(size);

  ext2_inode_t *root, *file;

  //create the root directory
  root = ext2_create_dir(__create_root__(), "/");
  ext2_root = root;

  ext2_path = (char*)kmalloc(2);
  *(ext2_path) = '/';
  *(ext2_path + 1) = 0;

  ext2_add_hardlink_to_dir(root, root, "."); //adds hardlink to root
  ext2_add_hardlink_to_dir(root, root, ".."); //adds hardlink to root
  ext2_add_hardlink_to_dir(root, root, "ppoopp");

  //~ file = ext2_create_file(root, "file_que_gana", 2 * EXT2_BLOCK_SZ);
  //~ ext2_add_file_to_dir(root, file, "file_que_es_nuevo");
  
  file = ext2_create_dir(root, "winner");

  struct ext2_dirent *test;

  test = ext2_dirent_from_dir(root, 3);

  k_printf("the magic name is: %s\n", test->name);

  ext2_set_current_dir(file); 

  k_printf("freeing\n");
  kfree(test->name);
  kfree(file);
  
}
