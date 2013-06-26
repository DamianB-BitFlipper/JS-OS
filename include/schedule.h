/*
 * schedule.h
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

#ifndef SCHEDULE
#define SCHEDULE

#include <system.h>

/*a pointer to a function that rearranges the tasks in some order*/
void (*rearrange_schedule)(void);

/*our scheduling algorithm*/
void schedule();

/*****************************
 * algorthms table contents  *
 * 0-prioritized round robin *
 * 1-prioritized SJF         *
/*****************************/
void set_scheduling_algorithm(u32int algorithm_number);

///scheduling algorithms///

/*arranges the tasks in the order of a prioritzed SJF (shorstest job first)*/
void sjf_arrange();

/*puts a task in its propper location in the list when initilalizing the task*/
void preempt_task(task_t *task_to_preempt);

#endif
