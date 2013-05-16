/*
 * timer.h
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

#ifndef TIMER_H
#define TIMER_H

#include <system.h>

typedef struct{
  u8int sec;
  u8int min;
  u8int hour;
  u8int day;
  u8int month;
  u8int year;
}datetime_t;

extern u32int globalFreq;

/*initialize the timer*/
void init_timer(u32int frequency);

/*sleep function in seconds*/
void sleep(u32int seconds);

/*sleep function in milliseconds*/
void mSleep(u32int milliseconds);

/*gets the RTC clock time*/
datetime_t getDatetime();

#endif //TIMER_H
