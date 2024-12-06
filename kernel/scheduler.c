#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"

// Maybe this should be placed in param.h
// but I would argue this is a better locality
// because the table is what really defines this
// number.
#define PRIORITIES_NUM 12

extern Proc_list procs;
extern struct spinlock procs_lock;

struct proc_queue queues[PRIORITIES_NUM];

// Sticking to Solaris tradition - higher
// index means higher priority (so the higher -
// the more important the process is)

// Pri.     Quantums    Pri. on yield   Pri. on wakeup
// 11          1             11                11        \   System Threads
// ...                                                   | - (just initproc 
// 8                                                    /    currently)
// ------
// 7                                                    \     Userspace 
// ...                                                  |  -  threads
// 0                                                   /
const struct feedback_row feedback[] = {
    {8, 0, 4},
    {8, 0, 4},
    {6, 1, 5},
    {6, 2, 5},
    {4, 2, 6},
    {4, 3, 6},
    {2, 4, 6},
    {2, 5, 7},
    {1, 11, 11},
    {1, 11, 11},
    {1, 11, 11},
    {1, 11, 11}
};



void
scheduler_init(void){
    for(int i = 0; i < PRIORITIES_NUM; ++i){
        initlock(&queues[i].q_lock, "sched_queue");
        lst_init(&queues[i].queue);
    }
}

void
scheduler_enqueue(struct proc* p, uint16 pri){

}

// Solaris-like Feedback FCFS
// multiprocess scheduler
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();

  c->proc = 0;
  for(;;){
    // The most recent process to run may have had interrupts
    // turned off; enable them to avoid a deadlock if all
    // processes are waiting.
    intr_on();

    int found = 0;
    acquire(&procs_lock);
    p = (struct proc*)procs.next;
    for (; (Proc_list *)p != &procs; p = (struct proc *)p->node.next) {
      acquire(&p->lock);
      if(p->state == RUNNABLE) {
        // Pseudo-LRU
        lst_rotate(&procs);

        release(&procs_lock);
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
        found = 1;
        release(&p->lock);
        break;
      }
      release(&p->lock);
    }
    if(found == 0) {
      release(&procs_lock);
      // nothing to run; stop running on this core until an interrupt.
      intr_on();
      asm volatile("wfi");
    }
  }
}