#!/bin/bash

echo 'nasm'
nasm -g -f elf32 -o boot.o boot.asm
nasm -g -f elf32 -o interrupt.o interrupt.asm
nasm -g -f elf32 -o gdt.o gdt.asm

echo 'gcc'
gcc -g -m32 -elf -o main.o -c main.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include
gcc -g -m32 -elf -o k_stdio.o -c k_stdio.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o k_math.o -c k_math.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I/usr/include -I/usr/lib/gcc/x86_64-linux-gnu/4.6/include
gcc -g -m32 -elf -o common.o -c common.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o descriptor_tables.o -c descriptor_tables.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o isr.o -c isr.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o timer.o -c timer.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o keyboard.o -c keyboard.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o k_shell.o -c k_shell.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector
gcc -g -m32 -elf -o k_programs.o -c k_programs.c -nostdlib -nostdinc -fno-builtin -fno-stack-protector

echo 'linker'
ld -g -T link.ld -o kernel2 boot.o main.o k_stdio.o common.o interrupt.o gdt.o descriptor_tables.o isr.o timer.o k_math.o keyboard.o k_shell.o k_programs.o -melf_i386

#~ rm *.o
