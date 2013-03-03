#!/bin/bash

#~ harddisk_image="harddisk.img"
harddisk_image="floppy.img"

printf "\n--------------------------GDB------[in new terminal window]----"
printf "\ntarget remote localhost:1234"
printf "\nsymbol-file kernel.sym"
printf "\nbreak <filename.c> :<function or line to break at>"
printf "\nUse 'continue' instead of 'run' in gdb\n\n"

objcopy --only-keep-debug kernel kernel.sym

qemu-system-x86_64 -vga std -soundhw pcspk -s -S -fda "$harddisk_image"


