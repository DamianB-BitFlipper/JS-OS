/*
 * common.c
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

// Write a byte out to the specified port.
void outb(u16int port, u8int value)
{
  asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int inb(u16int port)
{
  u8int ret;
  asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

u16int inw(u16int port)
{
  u16int ret;
  asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

// Copy len bytes from src to dest.
void memcpy(u8int *dest, const u8int *src, u32int len)
{
  const u8int *sp = (const u8int *)src;
  u8int *dp = (u8int *)dest;
  for(; len != 0; len--) *dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(u8int *dest, u8int val, u32int len)
{
  u8int *temp = (u8int *)dest;
  
  for ( ; len != 0; len--)
  {
      *temp++ = val;

      //~ k_printf("\ntemp is: %h", *temp);
  }
}

// Compare two strings. Should return -1 if 
// str1 < str2, 0 if they are equal or 1 otherwise.
int strcmp(char *str1, char *str2)
{
  int i = 0;
  int failed = 0;
  while(str1[i] != '\0' && str2[i] != '\0')
  {
    if(str1[i] != str2[i])
    {
        failed = 1;
        break;
    }
    i++;
  }
  // why did the loop exit?
  if( (str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0') )
    failed = 1;

  return failed;
}

// Copy the NULL-terminated string src into dest, and
// return dest.
char *strcpy(char *dest, const char *src)
{
  do
  {
    *dest++ = *src++;
  }
  while (*src != 0);
}

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *strcat(char *dest, const char *src)
{
  while (*dest != 0)
  {
    *dest = *dest++;
  }

  do
  {
    *dest++ = *src++;
  }
  while (*src != 0);
  return dest;
}

int strlen(char *src)
{
  int i = 0;
  while (*src++)
    i++;
  return i;
}

extern void panic(const char *message, const char *file, u32int line)
{
  // We encountered a massive problem and have to stop.
  asm volatile("cli"); // Disable interrupts.

  k_printf("PANIC( %s ) at %s:%d\n", message, file, line);
  //k_printf(message);
  //k_printf(") at ");
  //k_printf(file);
  //k_printf(":");
  //k_printf("%d", line);
  //k_printf("\n");
  // Halt by going into an infinite loop.
  for(;;);
}

extern void panic_assert(const char *file, u32int line, const char *desc)
{
  // An assertion failed, and we have to panic.
  asm volatile("cli"); // Disable interrupts.

  k_printf("ASSERTION-FAILED( %s ) at %s:%d\n", desc, file, line);
  //k_printf(desc);
  //k_printf(") at ");
  //k_printf(file);
  //k_printf(":");
  //k_printf("%d", line);
  //k_printf("\n");
  // Halt by going into an infinite loop.
  for(;;);
}
