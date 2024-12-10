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

static struct proc_entry*
proc_entry_alloc(void){
    struct proc_entry* e;
    if((e = (struct proc_entry*)bd_malloc(sizeof(struct proc_entry))) == 0){
        panic("proc_entry_alloc");
    }
    lst_init(&e->node);
    return e;
}

static void
proc_entry_free(struct proc_entry* e){
    bd_free(e);
}


void
scheduler_enqueue(struct proc* p, uint16 pri){
    struct proc_entry* e = proc_entry_alloc();
    e->p = p;
    acquire(&queues[pri].q_lock);
    lst_pushback(&queues[pri].queue, &e->node);
    release(&queues[pri].q_lock);
}

// Pops proc_entry with highest priority
// with p->lock acquired
// sets p->priotity
static struct proc_entry*
scheduler_next(void){
  for (int i = PRIORITIES_NUM - 1; i >= 0; --i) {

    struct proc_queue *pqueue = &queues[i];
    acquire(&pqueue->q_lock);

    struct proc_entry *entry = (struct proc_entry *)pqueue->queue.next;

    for (; (struct list *)entry != &pqueue->queue;
         entry = (struct proc_entry *)entry->node.next) {
      acquire(&entry->p->lock);

      if (entry->p->state == RUNNABLE) {
        // p->lock is not released
        lst_remove(&entry->node);
        release(&pqueue->q_lock);
        entry->p->priority = i;
        return entry;
      } else if (entry->p->state != SLEEPING)
        panic("pop_entry");

      release(&entry->p->lock);
    }
    release(&pqueue->q_lock);
  }
  return 0;
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
  for (;;) {
    // The most recent process to run may have had interrupts
    // turned off; enable them to avoid a deadlock if all
    // processes are waiting.
    intr_on();
    struct proc_entry *e = scheduler_next();
    if (!e) {
      // nothing to run; stop running on this core until an interrupt.
      intr_on();
      asm volatile("wfi");
      continue;
    }

    p = e->p;
    proc_entry_free(e);
    // Switch to chosen process.  It is the process's job
    // to release its lock and then reacquire it
    // before jumping back to us.
    p->state = RUNNING;
    p->quantums = feedback[p->priority].quantums;

    c->proc = p;
    swtch(&c->context, &p->context);

    if (p->state == RUNNABLE) {
      scheduler_enqueue(p, feedback[p->priority].tqexp);
    } else if (p->state == SLEEPING) {
      scheduler_enqueue(p, feedback[p->priority].slpret);
    }
    // Process is done running for now.
    // It should have changed its p->state before coming back.
    c->proc = 0;
    release(&p->lock);
  }
}