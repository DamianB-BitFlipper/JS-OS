/*
 * syscall.c
 * 
 * Copyright 2013 JS-OS <js@duck-squirell>
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

// syscall.c -- Defines the implementation of a system call system.
//              Based on code from JamesM's kernel development tutorials.

#include <system.h>

static void syscall_handler(registers_t *regs);

DEFN_SYSCALL1(user_printf, 0, const char*);
DEFN_SYSCALL1(user_putChar, 1, const char);
//~ DEFN_SYSCALL1(monitor_write_dec, 2, const char*);

static void *syscalls[2] =
{
  &k_printf,
  &k_putChar

};

u32int num_syscalls = 2;

void initialise_syscalls()
{
  // Register our syscall handler.
  register_interrupt_handler (0x80, &syscall_handler);
}

void syscall_handler(registers_t *regs)
{
  // Firstly, check if the requested syscall number is valid.
  // The syscall number is found in EAX.
  if (regs->eax >= num_syscalls)
    return;

  // Get the required syscall location.
  void *location = syscalls[regs->eax];

  // We don't know how many parameters the function wants, so we just
  // push them all onto the stack in the correct order. The function will
  // use all the parameters it wants, and we can pop them all back off afterwards.
  int ret;
  asm volatile (" \
    push %1; \
    push %2; \
    push %3; \
    push %4; \
    push %5; \
    call *%6; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
  " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
  regs->eax = ret;
}
