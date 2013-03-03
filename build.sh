#!/bin/bash

echo 'nasm'
nasm -g -f elf32 -o boot.o boot.asm
nasm -g -f elf32 -o interrupt.o interrupt.asm
nasm -g -f elf32 -o gdt.o gdt.asm
nasm -g -f elf32 -o process.o process.asm
nasm -g -f elf32 -o v86.o v86.asm

echo 'gcc'
gcc -g -m32 -elf -o main.o -c main.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include
#~ gcc -g -m32 -elf -o monitor.o -c monitor.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o k_stdio.o -c k_stdio.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o k_math.o -c k_math.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o k_shell.o -c k_shell.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o k_programs.o -c k_programs.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o keyboard.o -c keyboard.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o sound.o -c sound.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o x_server.o -c x_server.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o mouse.o -c mouse.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o graphics.o -c graphics.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
#gcc -g -m32 -elf -o k_math.o -c k_math.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o common.o -c common.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o descriptor_tables.o -c descriptor_tables.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o isr.o -c isr.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o timer.o -c timer.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
#gcc -g -m32 -elf -o keyboard.o -c keyboard.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
#gcc -g -m32 -elf -o k_shell.o -c k_shell.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o fs.o -c fs.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o initrd.o -c initrd.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o kheap.o -c kheap.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o ordered_array.o -c ordered_array.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o paging.o -c paging.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o syscall.o -c syscall.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o task.o -c task.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
#~ gcc -g -m32 -elf -o emu.o -c emu.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
#~ gcc -g -m32 -elf -o vbochsvbe.o -c vbochsvbe.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
#~ gcc -g -m32 -elf -o vga.o -c vga.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o vesa.o -c vesa.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector

echo 'linker'
ld -g -T link.ld -o kernel boot.o main.o k_stdio.o k_shell.o k_programs.o k_math.o keyboard.o sound.o x_server.o mouse.o graphics.o common.o interrupt.o gdt.o descriptor_tables.o isr.o timer.o process.o fs.o initrd.o kheap.o ordered_array.o paging.o syscall.o task.o v86.o vesa.o -melf_i386

#~ rm *.o
