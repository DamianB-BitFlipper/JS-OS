/*
 * system.h
 * 
 * Copyright 2013 JS <js@duck-squirell>
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


//#ifndef SYSTEM_H
//#define SYSTEM_H

/////MAY BE USEFUL
///*ls function that prints all header files formated to #include <__name__>
 //* ls -1 *.h | sed 's/^/#include </' | sed 's/$/>/'
 //*
 //*the alphabet, may be useful in ordering the headers
 //* A B C D E F G H I J K L M N O P Q R S T U V W X Y Z */
/////MAY BE USEFUL

//All of the include for this OS
#include "arch/common.h"
#include "arch/cpu.h"
#include "arch/descriptor_tables.h"
#include "elf.h"
#include "ext2_fs.h"
#include "drivers/fdc.h"
#include "gui/graphics.h"
#include "initrd.h"
#include "arch/isr.h"
#include "keyboard.h"
#include "arch/kheap.h"
#include "math/k_math.h"
#include "k_programs.h"
#include "k_shell.h"
#include "k_stdio.h"
#include "mouse.h"
#include "multiboot.h"
#include "ordered_array.h"
#include "arch/paging.h"
#include "schedule.h"
#include "sound.h"
#include "stdio.h"
#include "syscall.h"
#include "arch/task.h"
#include "arch/timer.h"
#include "system/version.h"
#include "drivers/vesa.h"
#include "vfs.h"
#include "gui/x_server.h"


//#endif //SYSTEM_H
