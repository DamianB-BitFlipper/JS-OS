/*
 * stdbool.h
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

#ifndef STDC__STDBOOL_H
#define STDC__STDBOOL_H

#ifdef __cplusplus
/* C++ has "bool" as a built-in type. This extension will add support for _Bool, TRUE and FALSE. */
#define _Bool bool
#else
typedef unsigned char _Bool;
typedef _Bool bool;
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#endif /* STDC__STDBOOL_H */