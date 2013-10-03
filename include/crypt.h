/*
 * crypt.h
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

#ifndef CRYPT_H
#define CRYPT_H

#include <system.h>

//the DES encryption's meta data
typedef struct des_header 
{
  u32int size;
  u32int magic;
} des_header_t;

/*for encryption using Ceaser shift*/
u8int *en_ceaser_shift(u8int *data, u32int size_bytes, u8int shift);

/*for decryption using Ceaser shift*/
u8int *de_ceaser_shift(u8int *data, u8int orig_shift);

/*for encryption using Vigenere cipher*/
u8int *en_vigenere_cipher(u8int *data, u32int size_bytes, char *key);

/*for decryption using Vigenere cipher*/
u8int *de_vigenere_cipher(u8int *data, char *key);

/*for encryption using the bitwise xor cipher*/
u8int *en_bitwise_xor(u8int *data, u32int size_bytes, char *pass_phrase);

/*for decryption using the bitwise xor cipher*/
u8int *de_bitwise_xor(u8int *data, char *pass_phrase);

/*for encryption using the DES cipher*/
u8int *en_DES_cipher(u8int *input, u32int size_bytes, char *pass_phrase);

/*for decryption using the DES cipher*/
u8int *de_DES_cipher(u8int *input, char *pass_phrase);

#endif
