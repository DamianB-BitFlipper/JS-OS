#!/bin/bash

harddisk_image="harddisk.img"

printf "\n--------------------------GDB------[in new terminal window]----"
printf "\ntarget remote localhost:1234"
printf "\nsymbol-file kernel.sym"
printf "\nbreak <filename.c> :<function or line to break at>"
printf "\nUse 'continue' instead of 'run' in gdb\n\n"

objcopy --only-keep-debug kernel2 kernel.sym

qemu-system-x86_64 -s -S -hda "$harddisk_image"


