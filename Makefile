#  Makefile
#
#  Copyright 2013 JS <js@duck-squirell>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
#

#Macros

#output file
OUTPUT = kernel

#object files
OBJ_DIR = object_files
C_OBJ_DIR = $(OBJ_DIR)/c
NASM_OBJ_DIR = $(OBJ_DIR)/nasm

OBJECTS = $(NASM_OBJ_DIR)/boot.o $(NASM_OBJ_DIR)/gdt.o $(NASM_OBJ_DIR)/interrupt.o \
$(NASM_OBJ_DIR)/process.o $(NASM_OBJ_DIR)/v86.o \
\
$(C_OBJ_DIR)/common.o $(C_OBJ_DIR)/cpu.o $(C_OBJ_DIR)/descriptor_tables.o \
$(C_OBJ_DIR)/isr.o $(C_OBJ_DIR)/kheap.o $(C_OBJ_DIR)/paging.o $(C_OBJ_DIR)/task.o \
$(C_OBJ_DIR)/timer.o \
\
$(C_OBJ_DIR)/graphics.o $(C_OBJ_DIR)/x_server.o \
\
$(C_OBJ_DIR)/k_math.o \
\
$(C_OBJ_DIR)/elf.o $(C_OBJ_DIR)/ext2_fs.o $(C_OBJ_DIR)/initrd.o $(C_OBJ_DIR)/keyboard.o \
$(C_OBJ_DIR)/k_programs.o $(C_OBJ_DIR)/k_shell.o $(C_OBJ_DIR)/k_stdio.o $(C_OBJ_DIR)/main.o \
$(C_OBJ_DIR)/mouse.o $(C_OBJ_DIR)/ordered_array.o $(C_OBJ_DIR)/schedule.o $(C_OBJ_DIR)/sound.o \
$(C_OBJ_DIR)/syscall.o $(C_OBJ_DIR)/vfs.o \
\
$(C_OBJ_DIR)/fdc.o \
\
$(C_OBJ_DIR)/vesa.o
#object files

#gcc flags
CC = @gcc

CC_MESAGE = gcc
C_FLAGS =  -g -m32 -elf
C_FINAL_FLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector
C_INCLUDES =  -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include -I./include

#nasm flags
NASM = @nasm
NASM_MESSAGE = NASM
NASM_FLAGS =  -g -f elf32

#linker flags
LD = @ld
LD_MESSAGE = ld
LD_MESSAGE_CONTENTS = linking...
LD_DONE = done\n
LD_FLAGS = -g -T link.ld
LD_FINAL_FLAGS = -melf_i386

#messages
MESSAGE_PRINTER = @printf
DONE = \n

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
				$(MESSAGE_PRINTER) "[$(LD_MESSAGE) ] $(LD_MESSAGE_CONTENTS)"
				$(LD) $(LD_FLAGS) -o $(OUTPUT) $^ $(LD_FINAL_FLAGS)
				$(MESSAGE_PRINTER) "$(LD_DONE)"

###********core/_C********###
$(C_OBJ_DIR)/main.o: core/main.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

###********core/_C********###


###********core/arch/_NASM********###
$(NASM_OBJ_DIR)/boot.o: core/arch/boot.asm
				$(MESSAGE_PRINTER) "[$(NASM_MESSAGE) ] $^"
				$(NASM) $(NASM_FLAGS) -o $@ $^
				$(MESSAGE_PRINTER) "$(DONE)"

$(NASM_OBJ_DIR)/gdt.o: core/arch/gdt.asm
				$(MESSAGE_PRINTER) "[$(NASM_MESSAGE) ] $^"
				$(NASM) $(NASM_FLAGS) -o $@ $^
				$(MESSAGE_PRINTER) "$(DONE)"

$(NASM_OBJ_DIR)/interrupt.o: core/arch/interrupt.asm
				$(MESSAGE_PRINTER) "[$(NASM_MESSAGE) ] $^"
				$(NASM) $(NASM_FLAGS) -o $@ $^
				$(MESSAGE_PRINTER) "$(DONE)"

$(NASM_OBJ_DIR)/process.o: core/arch/process.asm
				$(MESSAGE_PRINTER) "[$(NASM_MESSAGE) ] $^"
				$(NASM) $(NASM_FLAGS) -o $@ $^
				$(MESSAGE_PRINTER) "$(DONE)"

$(NASM_OBJ_DIR)/v86.o: core/arch/v86.asm
				$(MESSAGE_PRINTER) "[$(NASM_MESSAGE) ] $^"
				$(NASM) $(NASM_FLAGS) -o $@ $^
				$(MESSAGE_PRINTER) "$(DONE)"

###********core/arch/_NASM********###

###********core/arch/_C********###
$(C_OBJ_DIR)/common.o: core/arch/common.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/cpu.o: core/arch/cpu.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/descriptor_tables.o: core/arch/descriptor_tables.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/isr.o: core/arch/isr.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/kheap.o: core/arch/kheap.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/paging.o: core/arch/paging.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/task.o: core/arch/task.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/timer.o: core/arch/timer.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

###********core/arch/_C********###

###********core/gui/_C********###
$(C_OBJ_DIR)/graphics.o: core/gui/graphics.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/x_server.o: core/gui/x_server.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

###********core/gui/_C********###

###********core/math/_C********###
$(C_OBJ_DIR)/k_math.o: core/math/k_math.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

###********core/math/_C********###

###********core/_C********###
$(C_OBJ_DIR)/elf.o: core/elf.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/ext2_fs.o: core/ext2_fs.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/initrd.o: core/initrd.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/keyboard.o: core/keyboard.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/k_programs.o: core/k_programs.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/k_shell.o: core/k_shell.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/k_stdio.o: core/k_stdio.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/mouse.o: core/mouse.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/ordered_array.o: core/ordered_array.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/schedule.o: core/schedule.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/sound.o: core/sound.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/syscall.o: core/syscall.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

$(C_OBJ_DIR)/vfs.o: core/vfs.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"				

###********core/_C********###

###********drivers/floppy/_C********###
$(C_OBJ_DIR)/fdc.o: drivers/floppy/fdc.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

###********drivers/floppy/_C********###

###********drivers/video/_C********###
$(C_OBJ_DIR)/vesa.o: drivers/video/vesa.c
				$(MESSAGE_PRINTER) "[$(CC_MESAGE) ] $^"
				$(CC) $(C_FLAGS) -o $@ -c $^ $(C_FINAL_FLAGS) $(C_INCLUDES)
				$(MESSAGE_PRINTER) "$(DONE)"

###********drivers/video/_C********###
