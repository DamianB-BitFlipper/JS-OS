/*
 * stddef.h
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

#ifndef STDC__STDDEF_H
#define STDC__STDDEF_H

#undef __x86_64__
#define __i386__ (1)

#ifdef __cplusplus
#ifndef NULL
#define NULL (0)
#endif
#else
#ifndef NULL
#define NULL ((void*)0)
#endif
#endif

#endif /* STDC__STDDEF_H */