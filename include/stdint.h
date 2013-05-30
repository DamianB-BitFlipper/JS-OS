/*
 * stdint.h
 * 
 * Copyright 2013 Angel Perez <anpep.cc@gmail.com>
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

#ifndef STDC__STDINT_H
#define STDC__STDINT_H

/* Exact-width integer types. */
typedef signed char								int8_t;
typedef signed short							int16_t;
typedef signed int								int32_t;
#ifdef __x86_64__
typedef signed long								int64_t;
#else
typedef signed long long					int64_t;
#endif

typedef unsigned char							uint8_t;
typedef unsigned short						uint16_t;
typedef unsigned int							uint32_t;
#ifdef __x86_64__
typedef unsigned long							uint64_t;
#else
typedef unsigned long long				uint64_t;
#endif


#endif /* STDC__STDINT_H */