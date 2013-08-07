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

//functions beginning with en_ are to be used for encryption
//functions beginning with de_ are to be used for decryption

u8int *en_ceaser_shift(u8int *data, u32int size_bytes, u8int shift)
{
  //there is no point of encrypting it with a shift of 0
  if(!shift)
    return 0;

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

u8int *en_vigenere_cipher(u8int *data, u32int size_bytes, char *key)
{
  //if there is no key, there is no way to encrypt
  if(!key)
    return 0;

  u8int *encrypt;
  encrypt = (u8int*)kmalloc(size_bytes);

  u32int i, p = 0, key_len = strlen(key);
  for(i = 0; i < size_bytes; i++, p++)
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

    *(decrypt + i) = (*(data + i) + *(key + p)) % 256; 

  }

  return decrypt;
}
