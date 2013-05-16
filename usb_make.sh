#  usb_make.sh
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

#!/bin/bash

#check the first argument

GRUB_LOCATION="/grub-boot"
KERNEL="kernel"
INITRD="./initrd/initrd.img"

case "$1" in

  #user has input drive path
  -d)
    if [ -d "$2" ] #test if arg 2 directory exists
    then
     #remove any /boot directory on the usb, if it exits
     if [ -d "$2/boot/" ]
     then
         rm -rf $2/boot/
      fi

      #copy grub
      cp -r ./$GRUB_LOCATION/* "$2"
      #copy the kernel and initrd
      cp $KERNEL $2/boot
      cp $INITRD $2/boot
    else
      echo "$2 not found or is not a directory"
    fi;;

  #user wants to print the contents of /media/
  -p)
    ls /media/;;
  #user wants to print the license information
  -l)
    printf "Consult the file LICENCE in the directory ./README for more information\n";;

  #user wants help
  -h)
    printf "Usage:\n\t-d <drive location> (e.g., -d /media/<USB_LABEL>)\n"
    printf "\t-p prints the files in /media/ for convenience\n"
    printf "\t-l prints the license information for the use of this script\n"
    printf "\t-h prints this help\n";;

esac
