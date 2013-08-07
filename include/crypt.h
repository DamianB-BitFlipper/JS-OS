/*
 * k_shell.h
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

/*for encrypting using Ceaser shift*/
u8int *en_ceaser_shift(u8int *data, u32int size_bytes, u8int shift);

/*for decrypting using Ceaser shift*/
u8int *de_ceaser_shift(u8int *data, u8int orig_shift);

/*for encrypting using Vigenere cipher*/
u8int *en_vigenere_cipher(u8int *data, u32int size_bytes, char *key);

/*for decrypting using Vigenere cipher*/
u8int *de_vigenere_cipher(u8int *data, u32int size_bytes, char *key);

#endif
