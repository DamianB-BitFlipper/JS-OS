// timer.h -- Defines the interface for all PIT-related functions.
// Written for JamesM's kernel development tutorials.

#ifndef TIMER_H
#define TIMER_H

#include "common.h"

/*initialize the timer*/
void init_timer(u32int frequency);

/*sleep function in seconds*/
void sleep(int seconds);

/*sleep function in milliseconds*/
//~ void mSleep(long long int milliseconds);
void mSleep(int milliseconds);

#endif
