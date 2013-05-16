/*
 * cpu.c
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

#include <system.h>

unsigned char readCMOS(unsigned char addr)
{
  unsigned char ret;
  outb(0x70, addr);
  asm volatile("jmp 1f; 1: jmp 1f;1:");
  ret = inb(0x71);
  asm volatile("jmp 1f; 1: jmp 1f;1:");
  return ret;
}

void writeCMOS(unsigned char addr, unsigned int value)
{
  outb(0x70, addr);
  asm volatile("jmp 1f; 1: jmp 1f;1:");
  outb(0x71, value);
  asm volatile("jmp 1f; 1: jmp 1f;1:");
}

void reboot()
{
  outb(0x64, 0xFE); /* send reboot command */
}

void print_regs()
{
  registers_t *regs;
  asm ("movl %%eax, %0\n"
       "movl %%ebx, %1\n"
       "movl %%ecx, %2\n"
       "movl %%edx, %3\n"
       : "=m"(regs->eax), "=m"(regs->ebx), "=m"(regs->ecx), "=m"(regs->edx));
       
  //asm ("pop %%eax\n"
       //"movl %%esp, %1\n"
       //"movl %%ebp, %2\n"
       //"movl %%esi, %3\n"
       //"movl %%edi, %4\n"
       //: "=m"(regs->eip), "=m"(regs->esp), "=m"(regs->ebp)
       //, "=m"(regs->esi), "=m"(regs->esi));

  k_printf("eax: %h, ebx: %h, ecx: %h, edx: %h\n", regs->eax, regs->ebx
          , regs->ecx, regs->edx);
  //k_printf("eip: %h, esp: %h, ebp: %h, esi: %h, edi: %h\n", regs->eip, regs->esp
          //, regs->ebp, regs->edi);
}
