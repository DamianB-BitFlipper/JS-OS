/*  /\*
 *  * schedule.c
 *  *
 *  * Copyright 2013 JS-OS <js@duck-squirell>
 *  *
 *  * This program is free software; you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation; either version 2 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program; if not, write to the Free Software
 *  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  * MA 02110-1301, USA.
 *  *
 *  *
 *  *\/
 * 
 * #include <system.h>
 * 
 * //The currently running task.
 * extern volatile task_t *current_task;
 * 
 * //The start of the task linked list.
 * extern volatile task_t *ready_queue;
 * 
 * extern u32int nTasks;
 * 
 * /\*****************************
 *  * algorthms table contents  *
 *  * 0-prioritized round robin *
 *  * 1-prioritized SJF         *
 * /\*****************************\/
 * static u32int algorithm = 0; //set to default prioitized round robin
 * 
 * struct task_array
 * {
 *   struct task *process;
 * };
 * 
 * task_t *parent_of_task(task_t *src)
 * {
 *   task_t *tmp_task = (task_t*)ready_queue;
 * 
 *   while(tmp_task != 0)
 *   {
 *     if(tmp_task->next == src)
 *     {
 *       return tmp_task;
 *     }else{
 *       tmp_task = tmp_task->next;
 *     }
 *   }
 * 
 *   //error, did not find a parent
 *   return 0;
 * 
 * }
 * 
 * void swap_task_locations(task_t *dest, task_t *src)
 * {
 *   //gets the parents of the tasks
 *   task_t *parent_dest = parent_of_task(dest);
 *   task_t *parent_src = parent_of_task(src);
 * 
 *   //get the next tasks of these tasks
 *   struct task *src_next, *dest_next;
 *   src_next = src->next;
 *   dest_next = dest->next;
 * 
 *   //if the parent of dest is not 0 (not the first task)
 *   if(parent_dest != 0)
 *   {
 *     //the next task after the parent is assigned as source
 *     parent_dest->next = src;
 * 
 *     /\*the next task of src is changed to the next task of dest
 *      * if the dest_next is equal to the src, we set it to the dest
 *      * because if we set src->next = src, it will be an infinitly
 *      * recurvice list *\/
 *     src->next = src == dest_next ? dest : dest_next;
 *   }else{ //dest is the first task
 * 
 *     //we set the ready_queue to src
 *     ready_queue = src;
 * 
 *     /\*the next task of src is changed to the next task of dest
 *      * if the dest_next is equal to the src, we set it to the dest
 *      * because if we set src->next = src, it will be an infinitly
 *      * recurvice list *\/
 *     src->next = src == dest_next ? dest : dest_next;
 *   }
 * 
 *   //if the parent of src is not 0 (not the first task)
 *   if(parent_src != 0)
 *   {
 *     //the next task after the parent is assigned as dest
 *     //~ parent_src->next = dest;
 * 
 *     //the next task of dest is changed to the next task of src
 *     dest->next = src_next;
 *   }else{ //src is the first task
 * 
 *     //we set the ready_queue to dest
 *     ready_queue = dest;
 * 
 *     //the next task of dest is changed to the next task of src
 *     dest->next = src_next;
 *   }
 * 
 * }
 * 
 * void sjf_arrange()
 * {
 *   asm volatile("cli");
 * 
 *   //struct task_array *array_of_tasks;
 *   //array_of_tasks = (struct task_array*)kmalloc(nTasks * sizeof(struct task_array));
 * 
 *   //kfree(array_of_tasks);
 * 
 *   task_t *prev_task;
 *   task_t *cur_task;
 * 
 *   u32int i, a;
 *   u8int has_changed;
 * 
 *   for(a = 0; a < nTasks; a++)
 *   {
 *     //reset the values
 *     prev_task = (task_t*)ready_queue;
 *     cur_task = (task_t*)ready_queue->next;
 *     has_changed = FALSE;
 * 
 *     for(i = 0; i < nTasks - 1; i++) //-1 because we start of 1 task in, ready_queue->next as cur_task
 *     {
 *       if(cur_task->averaged_priority > prev_task->averaged_priority)
 *       {
 *         swap_task_locations(prev_task, cur_task);
 * 
 *         /\*here we try to skip 2 tasks ahead, since we got pushed back one
 *          * with the switch, we have a falesafe were if the next task is not
 *          * a zero, then it is ok to assign the next next value. If the next
 *          * value is 0, then we could risk a page fault as the next next value
 *          * does not exists*\/
 *         cur_task = cur_task->next == 0 ? 0 : cur_task->next->next;
 * 
 *         //we have changed the order in this pass
 *         has_changed = TRUE;
 *       }else{
 * 
 *         prev_task = prev_task->next;
 *         cur_task = cur_task->next;
 *       }
 * 
 *       //using this as a check, we are at the end
 *       if(cur_task == 0)
 *       {
 *         //exit the second for loop
 *         break;
 *       }
 *     }
 * 
 *     /\*we have go through the whole second loop without changing anything
 *      * thus we have sorted this task list, there is no need to continue
 *      * just exit *\/
 *     if(has_changed == FALSE)
 *     {
 *       break;
 *     }
 *   }
 * 
 *   asm volatile("sti");
 * }
 * 
 * void set_scheduling_algorithm(u32int algorithm_number)
 * {
 *   algorithm = algorithm_number;
 * 
 *   switch(algorithm_number)
 *   {
 *     case 0: //prioritized RR
 *       rearrange_schedule = 0;
 *       break;
 *     case 1: //prioritized SJF
 *       rearrange_schedule = sjf_arrange;
 *       break;
 *     default: //else, default to prioritized RR
 *       rearrange_schedule = 0;
 *       break;
 *   }
 * }
 * 
 * void preempt_task(task_t *task_to_preempt)
 * {
 *   asm volatile("cli");
 *   
 *   switch(algorithm)
 *   {
 *     // Add task_t *task_to_preempt to the end of the ready queue.
 *     case 0: //prioritized RR
 *     {
 *       // Find the end of the ready queue...
 *       task_t *tmp_task;
 *       tmp_task = (task_t*)ready_queue;
 *       while(tmp_task->next)
 *         tmp_task = tmp_task->next;
 * 
 *       // ...And extend it.
 *       tmp_task->next = task_to_preempt;
 *     
 *       break;
 *     }
 *     case 1: //prioritized SJF
 * 
 *       //the task_to_preempt is of a higher priority than the current task
 *       if(task_to_preempt->averaged_priority > current_task->averaged_priority)
 *       {
 *         task_t *cur_parent = parent_of_task((task_t*)current_task);
 * 
 *         if(current_task == ready_queue) //the current task is the ready_queue
 *         {
 *           //place the task_to_preempt directly infront of the current task and set it as current
 *           task_to_preempt->next = (task_t*)current_task;
 *           ready_queue = task_to_preempt;
 *           set_current_task(task_to_preempt);
 * 
 *         }else if(cur_parent != 0) //if the current task was not the ready queue, then we must have a parent
 *         {
 *           //set the next pointer of the parent to the task_to_preempt
 *           cur_parent->next = task_to_preempt;
 * 
 *           //then chain the current_task to the task_to_preempt
 *           task_to_preempt->next = (task_t*)current_task;
 * 
 *           //make the task_to_preempt the current task
 *           set_current_task(task_to_preempt);
 * 
 *         }
 * 
 *       }else if(task_to_preempt->averaged_priority <= current_task->averaged_priority)
 *       {
 *         //put task in order at the end
 *         task_t *tmp_task;
 *         tmp_task = (task_t*)current_task;
 * 
 *         /\*when we exit, if we exit sucessfully, the tmp_task is the
 *          * parent of where we have to place the task_to_preempt *\/
 *         for(;tmp_task->next->averaged_priority > task_to_preempt->averaged_priority; tmp_task = tmp_task->next)
 *         {
 *           if(!tmp_task)
 *           {
 *             asm volatile("sti");
 *             return;
 *           }
 * 
 *           if(!tmp_task->next)
 *           {
 *             tmp_task->next = task_to_preempt;
 *             task_to_preempt->next = 0;
 * 
 *             asm volatile("sti");
 * 
 *             return;
 *           }
 *         }
 * 
 *         //set the next of the task_to_preemt to the next from the parent (tmp_task)
 *         task_to_preempt->next = tmp_task->next;
 * 
 *         //set the parents next pointer to the task_to_preemt
 *         tmp_task->next = task_to_preempt;
 * 
 *         /\*we just inserted task_to_preemt between the parent (tmp_task)
 *          * and its child (tmp_task->next *\/
 *       }
 *       
 *       //break from the case statement
 *       break;
 * 
 *   }
 * 
 *   asm volatile("sti");
 * 
 * }
 * 
 * void schedule()
 * {
 *   if (!current_task->id)
 *     return;
 * 
 *   if(!current_task->ready_to_run)
 *     return;
 * 
 *   /\*current_task->time_to_run is not 0, that means that that task has been
 *    * preempted and interupted in the middle of its execution, so we should
 *    * not change its time to run and contiue with what it had left *\/
 *   if(!current_task->time_to_run)
 *   {
 * 
 *     switch(current_task->priority)
 *     {
 *       case PRIO_DEAD:
 *         current_task->time_to_run = 0; //0 milliseconds
 *         break;
 *       case PRIO_IDLE:
 *         //~ current_task->time_to_run = 10; //10 milliseconds
 *         current_task->time_to_run = 1; //1 millisecond
 *         break;
 *       case PRIO_LOW:
 *         //~ current_task->time_to_run = 100; //100 milliseconds
 *         current_task->time_to_run = 10; //10 milliseconds
 *         break;
 *       case PRIO_MED:
 *         //~ current_task->time_to_run = 200; //200 milliseconds
 *         current_task->time_to_run = 20; //20 milliseconds
 *         break;
 *       case PRIO_HIGH:
 *         //~ current_task->time_to_run = 500; //500 milliseconds
 *         current_task->time_to_run = 50; //50 milliseconds
 *         break;
 *       default:
 *         //~ current_task->time_to_run = 100; //100 milliseconds
 *         current_task->time_to_run = 10; //10 milliseconds
 *         break;
 *     }
 *   }
 * 
 *   //we are at the end of the tasks and there is a scheduling algorithm attached to rearrange_schedule
 *   if(!current_task->next && rearrange_schedule)
 *     rearrange_schedule();
 * 
 *   switch_task();
 * } */

/*
* schedule.c
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

//The currently running task.
extern volatile task_t *current_task;

//The start of the task linked list.
extern volatile task_t *ready_queue;

extern u32int nTasks;

/*****************************
* algorthms table contents *
* 0-prioritized round robin *
* 1-prioritized SJF *
/*****************************/
static u32int algorithm = 0; //set to default prioitized round robin

struct task_array
{
  struct task *process;
};

task_t *parent_of_task(task_t *src)
{
  task_t *tmp_task = (task_t*)ready_queue;

  while(tmp_task != 0)
  {
    if(tmp_task->next == src)
    {
      return tmp_task;
    }else{
      tmp_task = tmp_task->next;
    }
  }

  //error, did not find a parent
  return 0;

}

void swap_task_locations(task_t *dest, task_t *src)
{
  //gets the parents of the tasks
  task_t *parent_dest = parent_of_task(dest);
  task_t *parent_src = parent_of_task(src);

  //get the next tasks of these tasks
  struct task *src_next, *dest_next;
  src_next = src->next;
  dest_next = dest->next;

  //if the parent of dest is not 0 (not the first task)
  if(parent_dest != 0)
  {
    //the next task after the parent is assigned as source
    parent_dest->next = src;

    /*the next task of src is changed to the next task of dest
* if the dest_next is equal to the src, we set it to the dest
* because if we set src->next = src, it will be an infinitly
* recurvice list */
    src->next = src == dest_next ? dest : dest_next;
  }else{ //dest is the first task

    //we set the ready_queue to src
    ready_queue = src;

    /*the next task of src is changed to the next task of dest
* if the dest_next is equal to the src, we set it to the dest
* because if we set src->next = src, it will be an infinitly
* recurvice list */
    src->next = src == dest_next ? dest : dest_next;
  }

  //if the parent of src is not 0 (not the first task)
  if(parent_src != 0)
  {
    //the next task after the parent is assigned as dest
    //~ parent_src->next = dest;

    //the next task of dest is changed to the next task of src
    dest->next = src_next;
  }else{ //src is the first task

    //we set the ready_queue to dest
    ready_queue = dest;

    //the next task of dest is changed to the next task of src
    dest->next = src_next;
  }

}

void sjf_arrange()
{
  asm volatile("cli");

  //struct task_array *array_of_tasks;
  //array_of_tasks = (struct task_array*)kmalloc(nTasks * sizeof(struct task_array));

  //kfree(array_of_tasks);

  task_t *prev_task;
  task_t *cur_task;

  u32int i, a;
  u8int has_changed;

  for(a = 0; a < nTasks; a++)
  {
    //reset the values
    prev_task = (task_t*)ready_queue;
    cur_task = (task_t*)ready_queue->next;
    has_changed = FALSE;

    for(i = 0; i < nTasks - 1; i++) //-1 because we start of 1 task in, ready_queue->next as cur_task
    {
      if(cur_task->averaged_priority > prev_task->averaged_priority)
      {
        swap_task_locations(prev_task, cur_task);

        /*here we try to skip 2 tasks ahead, since we got pushed back one
* with the switch, we have a falesafe were if the next task is not
* a zero, then it is ok to assign the next next value. If the next
* value is 0, then we could risk a page fault as the next next value
* does not exists*/
        cur_task = cur_task->next == 0 ? 0 : cur_task->next->next;

        //we have changed the order in this pass
        has_changed = TRUE;
      }else{

        prev_task = prev_task->next;
        cur_task = cur_task->next;
      }

      //using this as a check, we are at the end
      if(cur_task == 0)
      {
        //exit the second for loop
        break;
      }
    }

    /*we have go through the whole second loop without changing anything
* thus we have sorted this task list, there is no need to continue
* just exit */
    if(has_changed == FALSE)
    {
      break;
    }
  }

  asm volatile("sti");
}

void set_scheduling_algorithm(u32int algorithm_number)
{
  algorithm = algorithm_number;

  switch(algorithm_number)
  {
    case 0: //prioritized RR
      rearrange_schedule = 0;
      break;
    case 1: //prioritized SJF
      rearrange_schedule = sjf_arrange;
      break;
    default: //else, default to prioritized RR
      rearrange_schedule = 0;
      break;
  }
}

void preempt_task(task_t *task_to_preempt)
{
  asm volatile("cli");
  
  switch(algorithm)
  {
    // Add task_t *task_to_preempt to the end of the ready queue.
    case 0: //prioritized RR
    {
      // Find the end of the ready queue...
      task_t *tmp_task;
      tmp_task = (task_t*)ready_queue;
      while(tmp_task->next != 0)
      {
        tmp_task = tmp_task->next;
    
      }
      
      // ...And extend it.
      tmp_task->next = task_to_preempt;
    
      break;
    }
    case 1: //prioritized SJF

      //the task_to_preempt is of a higher priority than the current task
      if(task_to_preempt->averaged_priority > current_task->averaged_priority)
      {
        task_t *cur_parent = parent_of_task((task_t*)current_task);

        if(current_task == ready_queue) //the current task is the ready_queue
        {
          //place the task_to_preempt directly infront of the current task and set it as current
          task_to_preempt->next = (task_t*)current_task;
          ready_queue = task_to_preempt;
          set_current_task(task_to_preempt);

        }else if(cur_parent != 0) //if the current task was not the ready queue, then we must have a parent
        {
          //set the next pointer of the parent to the task_to_preempt
          cur_parent->next = task_to_preempt;

          //then chain the current_task to the task_to_preempt
          task_to_preempt->next = (task_t*)current_task;

          //make the task_to_preempt the current task
          set_current_task(task_to_preempt);

        }

      }else if(task_to_preempt->averaged_priority <= current_task->averaged_priority)
      {
        //put task in order at the end
        task_t *tmp_task;
        tmp_task = (task_t*)current_task;

        /*when we exit, if we exit sucessfully, the tmp_task is the
* parent of where we have to place the task_to_preempt */
        for(;tmp_task->next->averaged_priority > task_to_preempt->averaged_priority; tmp_task = tmp_task->next)
        {
          if(!tmp_task)
          {
            asm volatile("sti");
            return;
          }

          if(!tmp_task->next)
          {
            tmp_task->next = task_to_preempt;
            task_to_preempt->next = 0;

            asm volatile("sti");

            return;
          }
        }

        //set the next of the task_to_preemt to the next from the parent (tmp_task)
        task_to_preempt->next = tmp_task->next;

        //set the parents next pointer to the task_to_preemt
        tmp_task->next = task_to_preempt;

        /*we just inserted task_to_preemt between the parent (tmp_task)
* and its child (tmp_task->next */
      }
      
      //break from the case statement
      break;

  }

  asm volatile("sti");

}

void schedule()
{
  if (!current_task->id)
    return;

  if(!current_task->ready_to_run)
    return;

  /*current_task->time_to_run is not 0, that means that that task has been
* preempted and interupted in the middle of its execution, so we should
* not change its time to run and contiue with what it had left */
  if(!current_task->time_to_run)
  {

    switch(current_task->priority)
    {
      case PRIO_DEAD:
        current_task->time_to_run = 0; //0 milliseconds
        break;
      case PRIO_IDLE:
        //~ current_task->time_to_run = 10; //10 milliseconds
        current_task->time_to_run = 1; //1 millisecond
        break;
      case PRIO_LOW:
        //~ current_task->time_to_run = 100; //100 milliseconds
        current_task->time_to_run = 10; //10 milliseconds
        break;
      case PRIO_MED:
        //~ current_task->time_to_run = 200; //200 milliseconds
        current_task->time_to_run = 20; //20 milliseconds
        break;
      case PRIO_HIGH:
        //~ current_task->time_to_run = 500; //500 milliseconds
        current_task->time_to_run = 50; //50 milliseconds
        break;
      default:
        //~ current_task->time_to_run = 100; //100 milliseconds
        current_task->time_to_run = 10; //10 milliseconds
        break;
    }
  }

  //we are at the end of the tasks and there is a scheduling algorithm attached to rearrange_schedule
  if(current_task->next == 0 && rearrange_schedule != 0)
  {
    rearrange_schedule();
  }

  switch_task();
}
