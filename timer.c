// timer.c -- Initialises the PIT, and handles clock updates.
// Written for JamesM's kernel development tutorials.

#include "timer.h"
#include "isr.h"

#include "k_math.h"
#include "k_stdio.h"

u32int tick = 0, globalFreq;
int tickTockSwitcher = 0;
int pass = 0, systemTimePassed = 0;
long long int secondsPassed = 0;

//~ int millisecondTime, run = TRUE;

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

//~ void timer_callback(registers_t regs)
void timer_callback()
{
  tick = (tick + 1) % (globalFreq + 1);
  systemTimePassed++;
  
  if(tick == globalFreq) //one second passed
  {
    secondsPassed++;

    //~ k_setprintf(15, 0, ""
    
    pass = (pass + 1) % 2;

    //~ if(tickTockSwitcher == 0)
    //~ {
      //~ k_printf("\rTick, with a clock frequency of:%d, Time elaplsed:%d", tick, secondsPassed);
    //~ }else if(tickTockSwitcher == 1)
    //~ {
      //~ k_printf("\rTock, with a clock frequency of:%d, Time elaplsed:%d", tick, secondsPassed);
    //~ }

    //~ tickTockSwitcher = math_abs(tickTockSwitcher - 1);
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
