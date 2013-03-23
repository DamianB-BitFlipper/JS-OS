// timer.h -- Defines the interface for all PIT-related functions.
// Written for JamesM's kernel development tutorials.

#ifndef TIMER_H
#define TIMER_H

#include "common.h"

typedef struct{
  unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char day;
  unsigned char month;
  unsigned char year;
}datetime_t;

/*initialize the timer*/
void init_timer(u32int frequency);

/*sleep function in seconds*/
void sleep(int seconds);

/*sleep function in milliseconds*/
void mSleep(int milliseconds);

/*gets the RTC clock time*/
datetime_t getDatetime();

#endif
