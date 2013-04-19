/*
 * timer.c
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

u32int tick = 0, globalFreq;
int tickTockSwitcher = 0;
int pass = 0, systemTimePassed = 0;
long long int secondsPassed = 0;

void sleep(int seconds)
{
  int onOrOff = pass;
  int timePassed = 0;
  
  while(timePassed != seconds)
  {
    if(onOrOff != pass)
    {
      timePassed++;
      onOrOff = pass;
    }
  }
}

//~ void mSleep(long long int milliseconds)
void mSleep(int milliseconds)
{
  ////~ long long int time = systemTimePassed;
  //if(run == TRUE)
  //{
    //millisecondTime = milliseconds + systemTimePassed;
    //run = FALSE;
  //}
  ////~ milliseconds = milliseconds + systemTimePassed;

  ////~ while(systemTimePassed != milliseconds)
  //if(systemTimePassed != millisecondTime)
  //{
    //mSleep(0);
  //}

  //if(millisecondTime == systemTimePassed)
  //{
    //run = TRUE;
  //}
  if(milliseconds > 0)
  {
    unsigned long eticks;

    eticks = systemTimePassed + milliseconds;
    while(systemTimePassed < eticks);
  }
  
}

long long int getSystemUpTime()
{
  return systemTimePassed;
}

void timer_callback()
{
  tick = (tick + 1) % (globalFreq + 1);
  systemTimePassed++;
  
  if(tick == globalFreq) //one second passed
  {
    secondsPassed++;

    pass = (pass + 1) % 2;

  }

  //Reduce the running tasks time in queue
  if(current_task->time_to_run > 0 && current_task->ready_to_run == TRUE)
  {
    current_task->time_to_run--;
    current_task->time_running++;
  }else{
    schedule(); //No time left
  }
  
}

void init_timer(u32int frequency)
{
  globalFreq = frequency;
  // Firstly, register our timer callback.
  register_interrupt_handler(IRQ0, &timer_callback);

  // The value we send to the PIT is the value to divide it's input clock
  // (1193180 Hz) by, to get our required frequency. Important to note is
  // that the divisor must be small enough to fit into 16-bits.
  u32int divisor = 1193180 / frequency;

  // Send the command byte.
  outb(0x43, 0x36);

  // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
  u8int l = (u8int)(divisor & 0xFF);
  u8int h = (u8int)( (divisor>>8) & 0xFF );

  // Send the frequency divisor.
  outb(0x40, l);
  outb(0x40, h);
}

/*================RTC===================*/

#define BCD2BIN(bcd) ((((bcd)&15) + ((bcd)>>4)*10))
#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

//Gets CMOS actual time
datetime_t getDatetime()
{
  datetime_t now;
  
  asm volatile("cli");
  now.sec = BCD2BIN(readCMOS(0x0));
  now.min = BCD2BIN(readCMOS(0x2));
  now.hour = BCD2BIN(readCMOS(0x4));
  now.day = BCD2BIN(readCMOS(0x7));
  now.month = BCD2BIN(readCMOS(0x8));
  now.year = BCD2BIN(readCMOS(0x9));
  asm volatile("sti");
  
  return now;
}
