/*
 * crypt.c
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

//!Very useful link for the DES encryption: http://orlingrabbe.com/des.htm

//functions beginning with en_ are to be used for encryption
//functions beginning with de_ are to be used for decryption

//TODO add meta data
u8int *en_ceaser_shift(u8int *data, u32int size_bytes, u8int shift)
{
  //there is no point of encrypting it with a shift of 0
  if(!shift)
  {
    k_printf("Error: Invalid shift passphrase\n");
    return 0;
  }

  //if there are no size bytes, we cannot continue
  if(!size_bytes)
  {
    k_printf("Error: File size is 0\n");
    return 0;
  }

  u8int *encrypt;
  encrypt = (u8int*)kmalloc(size_bytes);

  u32int i;
  for(i = 0; i < size_bytes; i++)
    *(encrypt + i) = (*(data + i) + shift) % 256; 

  return encrypt;
}

u8int *de_ceaser_shift(u8int *data, u8int orig_shift)
{
  //there is no point of decrypting if it was encrypted with a shift of 0
  if(!orig_shift)
    return 0;

  //there is no need to error check if orig_shift can be greater than 256 becuase of its typedef
  u32int size_bytes = size_of_alloc(data), shift = 256 - orig_shift;

  u8int *decrypt;
  decrypt = (u8int*)kmalloc(size_bytes);

  u32int i;
  for(i = 0; i < size_bytes; i++)
    *(decrypt + i) = (*(data + i) + shift) % 256; 

  return decrypt;
}

//TODO add meta data
u8int *en_vigenere_cipher(u8int *data, u32int size_bytes, char *key)
{
  //if there is no key, there is no way to encrypt
  if(!key)
    return 0;

  //if there are no size bytes, we cannot continue
  if(!size_bytes)
    return 0;

  u8int *encrypt;
  encrypt = (u8int*)kmalloc(size_bytes);

  u32int i, p, key_len = strlen(key);
  for(i = 0, p = 0; i < size_bytes; i++, p++)
  {
    //once p gets to the length of the key, reset it
    if(p == key_len)
      p = 0;

    *(encrypt + i) = (*(data + i) + *(key + p)) % 256; 

  }

  return encrypt;
}

u8int *de_vigenere_cipher(u8int *data, char *key)
{
  //if there is no key, there is no way to encrypt
  if(!key)
    return 0;

  u32int size_bytes = size_of_alloc(data);

  u8int *decrypt;
  decrypt = (u8int*)kmalloc(size_bytes);

  u32int i, p = 0, key_len = strlen(key);
  for(i = 0; i < size_bytes; i++, p++)
  {
    //once p gets to the length of the key, reset it
    if(p == key_len)
      p = 0;

    //reverse the encryption, the + 256 is to make
    *(decrypt + i) = ((*(data + i) + 256) - *(key + p)) % 256; 

  }

  return decrypt;
}

//TODO add meta data
u8int *en_bitwise_xor(u8int *data, u32int size_bytes, char *pass_phrase)
{
  //no password was entered, return with an error
  if(!pass_phrase)
    return 0;

  //if there are no size bytes, we cannot continue
  if(!size_bytes)
    return 0;

  u32int i, pass_len = strlen(pass_phrase), p = 0;

  //in the bitwise xor encryption, the input is the same size as the output
  u8int *encrypt;
  encrypt = (u8int*)kmalloc(size_bytes);

  for(i = 0; i < size_bytes; i++, p++)
  {
    if(p == pass_len)
      p = 0;

    *(encrypt + i) = *(data + i) ^ *(pass_phrase + p);

  }
  
  return encrypt;
}

u8int *de_bitwise_xor(u8int *data, char *pass_phrase)
{
  //no password was entered, return with an error
  if(!pass_phrase)
    return 0;

  u32int i, pass_len = strlen(pass_phrase), p = 0, size_bytes = size_of_alloc(data);

  //in the bitwise xor decryption, the input is the same size as the output
  u8int *decrypt;
  decrypt = (u8int*)kmalloc(size_bytes);

  for(i = 0; i < size_bytes; i++, p++)
  {
    if(p == pass_len)
      p = 0;

    *(decrypt + i) = *(data + i) ^ *(pass_phrase + p);

  }
  
  return decrypt;
}

/******The DES Encryption******/

#define DES_MAGIC 0xCA11AB1E

static u8int des_PC_1[56] =
{
  57,   49,    41,   33,    25,    17,    9,
  1,    58,    50,   42,    34,    26,   18,
  10,    2,    59,   51,    43,    35,   27,
  19,   11,     3,   60,    52,    44,   36,
  63,   55,    47,   39,    31,    23,   15,
  7,    62,    54,   46,    38,    30,   22,
  14,    6,    61,   53,    45,    37,   29,
  21,   13,     5,   28,    20,    12,    4
};

static u8int des_PC_2[48] = 
{
  14,    17,   11,    24,     1,    5,
  3,     28,   15,     6,    21,   10,
  23,    19,   12,     4,    26,    8,
  16,     7,   27,    20,    13,    2,
  41,    52,   31,    37,    47,   55,
  30,    40,   51,    45,    33,   48,
  44,    49,   39,    56,    34,   53,
  46,    42,   50,    36,    29,   32
};

static u8int des_left_shifts[16] =
{
  1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

static u8int des_ip[64] = 
{
  58,    50,   42,    34,    26,   18,    10,    2,
  60,    52,   44,    36,    28,   20,    12,    4,
  62,    54,   46,    38,    30,   22,    14,    6,
  64,    56,   48,    40,    32,   24,    16,    8,
  57,    49,   41,    33,    25,   17,     9,    1,
  59,    51,   43,    35,    27,   19,    11,    3,
  61,    53,   45,    37,    29,   21,    13,    5,
  63,    55,   47,    39,    31,   23,    15,    7
};

static u8int des_e_bit_selection[48] = 
{
  32,     1,    2,     3,     4,    5,
  4,      5,    6,     7,     8,    9,
  8,      9,   10,    11,    12,   13,
  12,    13,   14,    15,    16,   17,
  16,    17,   18,    19,    20,   21,
  20,    21,   22,    23,    24,   25,
  24,    25,   26,    27,    28,   29,
  28,    29,   30,    31,    32,    1
};

static u8int des_Sbox[8][64] = 
{
  {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
   0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
   4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
   15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13},

  {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
   3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
   0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
   13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9},

  {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
   13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
   13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
   1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12},

  {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
   13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
   10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
   3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14},

  {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
   14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
   4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
   11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3},

  {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
   10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
   9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
   4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13},

  {4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
   13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
   1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
   6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12},

  {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
   1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
   7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
   2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
};

u8int des_right_sub_perm[32] = 
{
  16, 7, 20, 21,
  29, 12, 28, 17,
  1, 15, 23, 26,
  5, 18, 31, 10,
  2, 8, 24, 14,
  32, 27, 3, 9,
  19, 13, 30, 6,
  22, 11, 4, 25
};


u8int des_final_perm[64] =
{
  40, 8, 48, 16, 56, 24, 64, 32,
  39, 7, 47, 15, 55, 23, 63, 31,
  38, 6, 46, 14, 54, 22, 62, 30,
  37, 5, 45, 13, 53, 21, 61, 29,
  36, 4, 44, 12, 52, 20, 60, 28,
  35, 3, 43, 11, 51, 19, 59, 27,
  34, 2, 42, 10, 50, 18, 58, 26,
  33, 1, 41,  9, 49, 17, 57, 25
};

static u8int *des_permutate_key(char *key)
{
  u32int pass_len = strlen(key);

  //the password length should be equal to exactly 8 bytes
  if(pass_len != 8)
    return 0;
  
  u8int *new_key;
  //allocate space for a 56 bit key
  new_key = (u8int*)kmalloc(56 / 8);
  memset(new_key, 0x0, 7);

  u32int i, byte = 0, new_byte;
  u8int bit = 0, bit_value = 0, new_bit;
  for(i = 0; i < 56; i++)
  {
    byte = des_PC_1[i] / 8;
    //-1 since des_PC_1 starts from 1, not 0
    bit = 0b10000000 >> (des_PC_1[i] - 1) % 8;

    //get the actually value of the bit
    bit_value = (*(key + byte) & bit) > 0 ? 1 : 0;

    //if the bit value is 0, there is no point since it already is a 0
    if(!bit_value)
      continue;

    new_byte = i / 8;
    new_bit = 0b10000000 >> (i % 8);

    *(new_key + new_byte) |= new_bit;

  }

  return new_key;
}

static u8int *des_permutate_sub_keys(u8int *a_key, u8int *b_key)
{
  u8int *new_key;
  //allocate space for a 48 bit key, bytes are easier to manipulate than bits
  new_key = (u8int*)kmalloc(48);

  u32int i;
  for(i = 0; i < 48; i++)
  {
    //the bit is in the first block, a_key
    if(des_PC_2[i] <= 28)
      //-1 since des_PC_2 starts counting from 1, not 0
      *(new_key + i) = *(a_key + des_PC_2[i] - 1);
    else
      //-28 since the 28th bit is the 1st of b_key, -1 because des_PC_2 starts from 1, not 0
      *(new_key + i) = *(b_key + (des_PC_2[i] - 28) - 1);
  }

  return new_key;
}

static void des_permutate_input(u8int *input, u8int *a_ip_key, u8int *b_ip_key)
{
  //none of the args should be 0
  if(!input)
    return;

  //clear the ip keys of any possible junk
  memset(a_ip_key, 0x0, 32);
  memset(b_ip_key, 0x0, 32);

  u32int i, byte = 0;
  u8int bit = 0, bit_value = 0;
  for(i = 0; i < 64; i++)
  {
    byte = (des_ip[i] - 1) / 8;
    bit = 0b10000000 >> (des_ip[i] - 1) % 8;

    //get the actually value of the bit
    bit_value = (*(input + byte) & bit) > 0 ? 1 : 0;

    //if the bit value is 0, there is no point since it already is a 0
    if(!bit_value)
      continue;
    //else, the bit_value will always be 1, and we have to set it

    //the bit is in the first block, a_ip_key
    if(i < 32)
      *(a_ip_key + i) = 1;
    else //the bit is in the second block
      *(b_ip_key + i - 32) = 1;
  }

  //sucess!
  return;
}

static u8int *des_shift_subkey_block(u8int *key, u32int shift)
{
  //if there is no shift return the input key
  if(!shift)
    return key;

  u32int i, b;
  u8int *out, end;
  //28 bits is the size fo one subkey block, every byte represents one bit
  out = (u8int*)kmalloc(28);

  u8int *tmp_key;
  tmp_key = key;

  for(i = 0; i < shift; i++)
  {
    if(i)
      tmp_key = out;

    //record the beggining bit that will go to the end
    end = *(tmp_key);

    //move all of the byte's values to the left one
    for(b = 0; b < 28; b++)
    {
      /*if b is at the last increment, manually assign out previously recorded 'end' value
       * moving the value of the first bit to the end*/
      if(b == 27)
        *(out + b) = end;
      else
        *(out + b) = *(tmp_key + b + 1);
    }
  }

  return out;
}

/*this is used to iterate the ip data of the des encryption*/
static void des_ip_f(u8int *ip_r, u8int *ip_l, u8int *old_r, u8int *subkey)
{
  u8int *e_ip_r, *s_box;
  e_ip_r = (u8int*)kmalloc(48);
  s_box = (u8int*)kmalloc(32);

  //for the s box transformation
  u8int beg_end, middle, byte;

  //do the permutation and xor
  u32int i, s = 0, a;
  for(i = 0; i < 48; i++)
  {
    //do the permutation into 48 bit from 32 bit
    //-1 since des_e_bit_selection starts counting from 1, not 0
    *(e_ip_r + i) = *(old_r + des_e_bit_selection[i] - 1);
      
    //do the xor with the subkey
    *(e_ip_r + i) ^= *(subkey + i);

    //if we have a group of 6 bits, then we can do the sbox transformations
    if((i % 6) == 5)
    {
      //the beginning bit shifted one left | the last bit
      beg_end = (*(e_ip_r + i - 5) << 1) | *(e_ip_r + i);

      //the 2nd bit shifted left 3 | 3rd bit shifted left 2 | 4th bit shifted left 1 | 5th bit
      middle =  (*(e_ip_r + i - 4) << 3) | (*(e_ip_r + i - 3) << 2)  | (*(e_ip_r + i - 2) << 1) |  *(e_ip_r + i - 1);

      //get the shrinked byte, (it was 6 bits, but after this operation it is 4 bits)
      //* 16, since the collums start from 0 to 15
      byte = des_Sbox[s][beg_end * 16 + middle];

      //set the byte into the u8int *s_box
      for(a = 0; a < 4; a++)
        //assign the propper bit to the propper place in s_box
        *(s_box + (s * 4) + a) = (byte >> (3 - a)) & 0b00000001;

      s++;
    }

  }

  //final permutate of the right subkey
  for(i = 0; i < 32; i++)
  {
    //-1 since des_right_sub_perm starts counting from 1, not 0    
    *(ip_r + i) = *(s_box + des_right_sub_perm[i] - 1);
  
    //xor the permutate ip_r with the ip_l, to get the new, final ip_l
    *(ip_r + i) = *(ip_l + i) ^ *(ip_r + i);
  } 

  kfree(e_ip_r);
  kfree(s_box);

  return;
}

u8int *en_DES_cipher(u8int *input, u32int size_bytes, char *pass_phrase)
{
  //no password was entered, return with an error
  if(!pass_phrase)
    return 0;

  //if there are no size bytes, we cannot continue
  if(!size_bytes)
    return 0;

  u32int pass_len = strlen(pass_phrase);

  //the password length must be 8 bytes
  if(pass_len != 8)
  {
    k_printf("Error: Password length is not 8 bytes long");
    return 0;
  }

  //copy the input to our own allocation
  u8int *data;
  data = (u8int*)kmalloc(size_bytes);
  memcpy(data, input, size_bytes);

  //our data's size has to be exactly a multiple of 64 bits (8 bytes) 
  if(size_bytes % 8)
  {
    //allocate more space to make the size a multiple of 64 bits (8 bytes)
    u8int *new_data;
    new_data = (u8int*)kmalloc(size_bytes + 8 - (size_bytes % 8));

    //copy the acutal data to our new buffer
    memcpy(new_data, data, size_bytes);

    //pad the rest of the space with 0's
    memset(new_data + size_bytes, 0x0, 8 - (size_bytes % 8));

    //free the old data
    kfree(data);

    //assign the new data
    data = new_data;
    size_bytes += (8 - (size_bytes % 8));
  }

  //allocate our output
  u8int *out;
  out = (u8int*)kmalloc(size_bytes);

  //clear the out of any possible junk
  memset(out, 0x0, size_bytes);

  //obtain the new des key
  u8int *des_key;
  des_key = des_permutate_key(pass_phrase);

  //split the new des key into 2 blocks
  u8int *a_key, *b_key;

  //we are technically supposed to allocate 7/2 (3.5) bytes, but maniplulating bytes is easier than bits
  a_key = (u8int*)kmalloc(28);
  b_key = (u8int*)kmalloc(28);

  //copy the first half of the des_key to the a_key and the second half to the b_key
  u32int byte, i;
  u8int bit;

  for(i = 0; i < 28; i++)
  {
    byte = i / 8;
    bit = 0b10000000 >> (i % 8);

    //copies every bit to a byte in the a_key, for easier manipulation
    *(a_key + i) = (*(des_key + byte) & bit) > 0 ? 1 : 0;
   
    if(i < 4) //for the trialing bits, i.e. in the 4th byte of des_key, the starting bit of b_key is the 5th bit
    {
      bit = 0b00001000 >> (i % 8);     
      *(b_key + i) = (*(des_key + byte + 3) & bit) > 0 ? 1 : 0;
    }else{  //for the non-trialing bits
      byte = (i - 4) / 8;
      bit = 0b10000000 >> ((i - 4) % 8);
      *(b_key + i) = (*(des_key + byte + 4) & bit) > 0 ? 1 : 0;
    }
  }

  //structure that will hold all of the subkeys
  u8int **a_sub_keys, **b_sub_keys, **permutate_keys;
  a_sub_keys = (u8int**)kmalloc(16 * sizeof(u8int*));
  b_sub_keys = (u8int**)kmalloc(16 * sizeof(u8int*));
  permutate_keys = (u8int**)kmalloc(16 * sizeof(u8int*));

  //permutate all of the other subkeys and create the final subkeys (permutate_keys)
  for(i = 0; i < 16; i++)
  {
    *(a_sub_keys + i) = des_shift_subkey_block(i == 0 ? a_key : *(a_sub_keys + i - 1), des_left_shifts[i]);
    *(b_sub_keys + i) = des_shift_subkey_block(i == 0 ? b_key : *(b_sub_keys + i - 1), des_left_shifts[i]);

    *(permutate_keys + i) = des_permutate_sub_keys(*(a_sub_keys + i), *(b_sub_keys + i));
  }

  //start the actual encryption of the text
  u32int b;
  u8int **a_ip_phrases, **b_ip_phrases, *ip_l, *ip_r, *tmp_l, *tmp_r, *ip_last;
  a_ip_phrases = (u8int**)kmalloc((size_bytes / 8) * sizeof(u8int*));
  b_ip_phrases = (u8int**)kmalloc((size_bytes / 8) * sizeof(u8int*));
  ip_l = (u8int*)kmalloc(32);
  ip_r = (u8int*)kmalloc(32);
  ip_last = (u8int*)kmalloc(64);

  /*used to store the old ip_l and ip_r data after the new data is written over it,
   * but the old data is needed for calculating the new ip_r*/
  tmp_l = (u8int*)kmalloc(32);
  tmp_r = (u8int*)kmalloc(32);
  for(i = 0; i < size_bytes / 8; i++)
  {
    //allocate the ip_phrases
    *(a_ip_phrases + i) = (u8int*)kmalloc(32);
    *(b_ip_phrases + i) = (u8int*)kmalloc(32);

    des_permutate_input(data + (i * 8), *(a_ip_phrases + i), *(b_ip_phrases + i));

    //clear the ip left and right buffers
    memset(ip_l, 0x0, 32);
    memset(ip_r, 0x0, 32);

    //copy the left and right buffers
    memcpy(ip_l, *(a_ip_phrases + i), 32);
    memcpy(ip_r, *(b_ip_phrases + i), 32);

    //iterating and encrypting the ip data
    //here, we iterate by L1 = R0 and R1 = L0 xor f(R0, K1)
    for(b = 0; b < 16; b++)
    {
      //store the old data of ip_l and ip_r before overwriting it
      memcpy(tmp_l, ip_l, 32);
      memcpy(tmp_r, ip_r, 32);

      //calculate the next iteration of ip_l, it is the previous ip_r
      memcpy(ip_l, ip_r, 32);

      //calculate the new ip_r
      des_ip_f(ip_r, tmp_l, tmp_r, *(permutate_keys + b));      
    }

    //when we exit we have out final iterated left and right blocks, reverse the 2 blocks and apply the final permutation
    for(b = 0; b < 64; b++)
    {
      //this should
      if(des_final_perm[b] <= 32)
        //purposly starting with the right since we ultimatly have to reverse the left and right blocks
        //-1 since des_final_perm starts form 1, not 0
        *(ip_last + b) = *(ip_r + des_final_perm[b] - 1);
      else
        //-1 since des_final_perm starts form 1, not 0
        *(ip_last + b) = *(ip_l + (des_final_perm[b] - 32) - 1);

    }

    //copy the bits from ip_last to the corresponding place in the output (u8int *out)
    for(b = 0; b < 64; b++)
    {
      //there is no need to set a bit to 0 if it alreadt is
      if(!*(ip_last + b))
        continue;

      byte = b / 8;
      bit = 0b10000000 >> (b % 8);
      *(out + i * 8 + byte) |= bit;
    }
  }

  //add the meta data to the final output
  u8int *final;
  final = (u8int*)kmalloc(size_bytes + sizeof(des_header_t));

  des_header_t *meta;
  meta = (des_header_t*)kmalloc(sizeof(des_header_t));
  meta->size = size_bytes;
  meta->magic = DES_MAGIC;
    
  memcpy(final, meta, sizeof(des_header_t));
  memcpy(final + sizeof(des_header_t), out, size_bytes);

  //free everything we kmalloc'd
  kfree(data);

  kfree(des_key);
  kfree(a_key);
  kfree(b_key);
  
  for(i = 0; i < 16; i++)
  {
    kfree(*(a_sub_keys + i));
    kfree(*(b_sub_keys + i));
    kfree(*(permutate_keys + i));
  }

  kfree(a_sub_keys);
  kfree(b_sub_keys);
  kfree(permutate_keys);

  for(i = 0; i < size_bytes / 8; i++)
  {
    kfree(*(a_ip_phrases + i));
    kfree(*(b_ip_phrases + i));
  }

  kfree(a_ip_phrases);
  kfree(b_ip_phrases);
  kfree(ip_l);
  kfree(ip_r);
  kfree(ip_last);
  kfree(tmp_l);
  kfree(tmp_r);

  kfree(out);
  kfree(meta);

  //done!
  return final;
}

//TODO, the size_of_alloc does not work all the time. :(

/*decryption is nealy identical as encryption, 
 * just that the order of the subkeys being applied is reversed*/
u8int *de_DES_cipher(u8int *input, char *pass_phrase)
{
  //no password was entered or there is no data, return with an error
  if(!pass_phrase || !input)
    return 0;
  
  u32int pass_len = strlen(pass_phrase);

  //the password length must be 8 bytes
  if(pass_len != 8)
  {
    k_printf("Error: Password length is not 8 bytes long");
    return 0;
  }

  //extract the meta data from the input
  des_header_t *meta;
  meta = (des_header_t*)kmalloc(sizeof(des_header_t));
  memcpy(meta, input, sizeof(des_header_t));

  if(meta->magic != DES_MAGIC)
  {
    k_printf("Error: Meta data is corrupted or not correct\n");
    return 0;
  }

  u8int *data;
  data = (u8int*)kmalloc(meta->size);
  memcpy(data, input + sizeof(des_header_t), meta->size);

  u32int size_bytes = meta->size;

  //our data's size has to be exactly a multiple of 64 bits (8 bytes), else it is an error
  if(size_bytes % 8)
    return 0;

  //allocate our output
  u8int *out;
  out = (u8int*)kmalloc(size_bytes);

  //clear the out of any possible junk
  memset(out, 0x0, size_bytes);

  //obtain the new des key
  u8int *des_key;
  des_key = des_permutate_key(pass_phrase);

  //split the new des key into 2 blocks
  u8int *a_key, *b_key;

  //we are technically supposed to allocate 7/2 (3.5) bytes, but maniplulating bytes is easier than bits
  a_key = (u8int*)kmalloc(28);
  b_key = (u8int*)kmalloc(28);

  //copy the first half of the des_key to the a_key and the second half to the b_key
  u32int byte, i;
  u8int bit;

  for(i = 0; i < 28; i++)
  {
    byte = i / 8;
    bit = 0b10000000 >> (i % 8);

    //copies every bit to a byte in the a_key, for easier manipulation
    *(a_key + i) = (*(des_key + byte) & bit) > 0 ? 1 : 0;
   
    if(i < 4) //for the trialing bits, i.e. in the 4th byte of des_key, the starting bit of b_key is the 5th bit
    {
      bit = 0b00001000 >> (i % 8);     
      *(b_key + i) = (*(des_key + byte + 3) & bit) > 0 ? 1 : 0;
    }else{  //for the non-trialing bits
      byte = (i - 4) / 8;
      bit = 0b10000000 >> ((i - 4) % 8);
      *(b_key + i) = (*(des_key + byte + 4) & bit) > 0 ? 1 : 0;
    }
  }

  //structure that will hold all of the subkeys
  u8int **a_sub_keys, **b_sub_keys, **permutate_keys; //, **poop;
  a_sub_keys = (u8int**)kmalloc(16 * sizeof(u8int*));
  b_sub_keys = (u8int**)kmalloc(16 * sizeof(u8int*));
  permutate_keys = (u8int**)kmalloc(16 * sizeof(u8int*));

  //permutate all of the other subkeys and create the final subkeys (permutate_keys)
  for(i = 0; i < 16; i++)
  {
    *(a_sub_keys + i) = des_shift_subkey_block(i == 0 ? a_key : *(a_sub_keys + i - 1), des_left_shifts[i]);
    *(b_sub_keys + i) = des_shift_subkey_block(i == 0 ? b_key : *(b_sub_keys + i - 1), des_left_shifts[i]);

    *(permutate_keys + i) = des_permutate_sub_keys(*(a_sub_keys + i), *(b_sub_keys + i));
  }

  //start the actual encryption of the text
  u32int b;
  u8int **a_ip_phrases, **b_ip_phrases, *ip_l, *ip_r, *tmp_l, *tmp_r, *ip_last;
  a_ip_phrases = (u8int**)kmalloc((size_bytes / 8) * sizeof(u8int*));
  b_ip_phrases = (u8int**)kmalloc((size_bytes / 8) * sizeof(u8int*));
  ip_l = (u8int*)kmalloc(32);
  ip_r = (u8int*)kmalloc(32);
  ip_last = (u8int*)kmalloc(64);
  
  /*used to store the old ip_l and ip_r data after the new data is written over it,
   * but the old data is needed for calculating the new ip_r*/
  tmp_l = (u8int*)kmalloc(32);
  tmp_r = (u8int*)kmalloc(32);
  
  for(i = 0; i < size_bytes / 8; i++)
  {
    //allocate the ip_phrases
    *(a_ip_phrases + i) = (u8int*)kmalloc(32);
    *(b_ip_phrases + i) = (u8int*)kmalloc(32);
  
    des_permutate_input(data + (i * 8), *(a_ip_phrases + i), *(b_ip_phrases + i));
    
    //clear the ip left and right buffers
    memset(ip_l, 0x0, 32);
    memset(ip_r, 0x0, 32);
    
    //copy the left and right buffers
    memcpy(ip_l, *(a_ip_phrases + i), 32);
    memcpy(ip_r, *(b_ip_phrases + i), 32);
    
    //iterating and encrypting the ip data
    //here, we iterate by L1 = R0 and R1 = L0 xor f(R0, K1)
    for(b = 0; b < 16; b++)
    {
      //store the old data of ip_l and ip_r before overwriting it
      memcpy(tmp_l, ip_l, 32);
      memcpy(tmp_r, ip_r, 32);
    
      //calculate the next iteration of ip_l, it is the previous ip_r
      memcpy(ip_l, ip_r, 32);
    
      //calculate the new ip_r
      /*notice here that the + 15 - b reverses the order of the subkeys being applied,
       * thus preforming the decryption instead of the encryption*/
      des_ip_f(ip_r, tmp_l, tmp_r, *(permutate_keys + 15 - b));      
    }
    
    //when we exit we have out final iterated left and right blocks, reverse the 2 blocks and apply the final permutation
    for(b = 0; b < 64; b++)
    {
      //this should
      if(des_final_perm[b] <= 32)
        //purposly starting with the right since we ultimatly have to reverse the left and right blocks
        //-1 since des_final_perm starts form 1, not 0
        *(ip_last + b) = *(ip_r + des_final_perm[b] - 1);
      else
        //-1 since des_final_perm starts form 1, not 0
        *(ip_last + b) = *(ip_l + (des_final_perm[b] - 32) - 1);
    
    }
    
    //copy the bits from ip_last to the corresponding place in the output (u8int *out)
    for(b = 0; b < 64; b++)
    {
      //there is no need to set a bit to 0 if it alreadt is
      if(!*(ip_last + b))
        continue;
    
      byte = b / 8;
      bit = 0b10000000 >> (b % 8);
      *(out + i * 8 + byte) |= bit;
    }
  }

  //free everything we kmalloc'd
  kfree(meta);
  kfree(data);

  kfree(des_key);
  kfree(a_key);
  kfree(b_key);
  
  for(i = 0; i < 16; i++)
  {
    kfree(*(a_sub_keys + i));
    kfree(*(b_sub_keys + i));
    kfree(*(permutate_keys + i));
  }

  kfree(a_sub_keys);
  kfree(b_sub_keys);
  kfree(permutate_keys);

  for(i = 0; i < size_bytes / 8; i++)
  {
    kfree(*(a_ip_phrases + i));
    kfree(*(b_ip_phrases + i));
  }

  kfree(a_ip_phrases);
  kfree(b_ip_phrases);
  kfree(ip_l);
  kfree(ip_r);
  kfree(ip_last);
  kfree(tmp_l);
  kfree(tmp_r);

  //done!
  return out;
}
