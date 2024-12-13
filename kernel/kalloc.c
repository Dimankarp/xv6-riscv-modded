// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

#define MAX_REFS  (uint8)(-1)
static uint8* pgrefs;
static struct spinlock pgrefs_lock;
static void* pgstart;

#define PG_INDEX(pa) (((pa) - pgstart)/PGSIZE)

void
kinit()
{
  initlock(&pgrefs_lock, "page_refs");
  pgstart = (void*) PGROUNDUP((uint64)end);

  bd_init(pgstart, (void *)(PHYSTOP)); // buddy allocator
  if((pgrefs = bd_malloc(sizeof(uint8) * bd_nblck(PGSIZE))) == 0)
    panic("kinit");
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&pgrefs_lock);

  uint8 *refs = &pgrefs[PG_INDEX(pa)];
  *refs -= 1;

  if (*refs == 0) {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);
    bd_free(pa);
  }
  release(&pgrefs_lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  void *page;
  page = bd_malloc(PGSIZE);

  if (page){
    memset((char *)page, 5, PGSIZE); // fill with junk

    // No lock required
    pgrefs[PG_INDEX(page)] = 1;
  }
  return page;
}

// Increments reference counter
// of a _pg_ page
void
krefinc(void* pg){

  if (((uint64)pg % PGSIZE) != 0 || (char *)pg < end || (uint64)pg >= PHYSTOP)
    panic("kfreeinc");

  acquire(&pgrefs_lock);
  uint8 *refs = &pgrefs[PG_INDEX(pg)];
  if (*refs == MAX_REFS)
    panic("kfreeinc: max refs");
  if (*refs == 0)
    panic("kfreeinc: inc free pg");

  *refs += 1;
  release(&pgrefs_lock);
}

// Is page shared by multiple processes
// returns 1 if reference counter of a _pg_
// page is > 1, otherwise returns 0
int
kisshared(void* pg){
    acquire(&pgrefs_lock);
    uint8 refs = pgrefs[PG_INDEX(pg)];
    release(&pgrefs_lock);
    return refs > 1;
}
