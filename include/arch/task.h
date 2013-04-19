/*
 * task.h
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

#ifndef TASK_H
#define TASK_H

#include <system.h>

#define KERNEL_STACK_SIZE     2048       // Use a 2kb kernel stack.

//process priorities
#define PRIO_DEAD             9900
#define PRIO_IDLE             0
#define PRIO_LOW              100
#define PRIO_MED              200
#define PRIO_HIGH             300

//process length times
#define PROC_VERY_LONG        10
#define PROC_LONG             30
#define PROC_MED              50
#define PROC_SHORT            70
#define PROC_VERY_SHORT       90

// This structure defines a 'task' - a process.
typedef struct task
{
  u32int id;                               //Process ID

  char name[32];                           //Process name
  
  u32int esp, ebp;                         //Stack and base pointers.
  u32int eip;                              //Instruction pointer.

  u32int priority;                         //the priority of the process
  u32int burst_time;                       //the process burst time
  u32int averaged_priority;                //A number that indicates average priority (priority and burst time)
  
  u32int time_to_run;                      //Time left on quanta
  u32int time_running;                     //Time spent running
  u32int ready_to_run;                     //is the process ready to run
  
  page_directory_t *page_directory;        // Page directory.
  
  void (*thread)();                        //thread entry point
  u32int thread_flags;                     //location of an array of thread arguments to go allong with that function
  
  u32int stack;                            //kernel stack location.
  u32int originalStack;                    //the original kernel stack location.
  
  struct task *next;                       //the next task in a linked list.
}task_t;

//The currently running task.
extern volatile task_t *current_task;

/*Initialises the tasking system*/
void initialise_tasking();

/*Called by the timer hook, this changes the running process to the next one in the list*/
void switch_task();

/*sets the input task as the current*/
void set_current_task(task_t *task_to_set);

/*Forks the current process, spawning a new one with a different memory space*/
u32int fork(u32int priority, u32int burst_time, char *task_Name);

/*Causes the current process' stack to be forcibly moved to a new location*/
void move_stack(void *new_stack_start, u32int size);

/*Returns the pid of the current process*/
int getpid();

/*kills a task*/
int kill_task(u32int pid);

/*start a task*/
u32int start_task(u32int priority, u32int burst_time, void (*func)(), void *arg, char *task_Name);

/*a test for the multitasking*/
void tasking_test();

/*end the current task*/
void exit();

#endif
