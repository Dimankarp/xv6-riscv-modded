#include "types.h"
#include "riscv.h"
#include "spinlock.h"
#include "list.h"
#include "shared.h"
#include "defs.h"


SharedSegs_list segments;
// helps synchronize operations on shared segments list (segments->node)
// must be acquired before any s->lock.
struct spinlock segments_lock;

int nextsegid = 0;
struct spinlock segid_lock;

static int
allocid()
{
  int id;
  
  acquire(&segid_lock);
  id = nextsegid;
  nextsegid = nextsegid + 1;
  release(&segid_lock);

  return id;
}

void
sharedinit(void)
{
  initlock(&segments_lock, "segments");
  initlock(&segid_lock, "nextsegid");
  lst_init(&segments);
}

// Allocates new shared segment of size _sz_
// rounded up to page sizes.
// returns pointer on success
// returns 0 on fail
struct shared_segment*
sharedalloc(uint64 sz){
  struct shared_segment *seg = bd_malloc(sizeof(struct shared_segment));

  initlock(&seg->lock, "sharedseg");
  seg->id = allocid();
  seg->refs = 1;
  lst_init(&seg->pages);

  uint64 npages = PGROUNDUP(sz) / PGSIZE;
  void *page;
  struct shared_page *shared;
  int i = 0;
  for (; i < npages; ++i) {
    if ((page = kalloc()) == 0) {
      goto err;
    }

    if ((shared = bd_malloc(sizeof(struct shared_page))) == 0) {
      kfree(page);
      goto err;
    }
    shared->pa = (uint64)page;
    lst_init(&shared->node);
    lst_pushback(&seg->pages, shared);
  }

  seg->pages_count = npages;
  acquire(&segments_lock);
  lst_push(&segments, seg);
  release(&segments_lock);

  return seg;
err:
  for (int j = 0; j < i; ++j) {
    struct shared_page *page =
        (struct shared_page *)lst_pop((struct list *)&seg->pages);
    kfree((void *)page->pa);
    bd_free(page);
  }
  bd_free(seg);
  return 0;
}

// Decrements ref counter of a _shared_
// segment and frees it when this counter
// reaches 0.
void
sharedfree(struct shared_segment* shared){

  acquire(&shared->lock);

  if (shared->refs == 0)
    panic("sharedfree");

  if (shared->refs > 1) {
    shared->refs--;
    release(&shared->lock);
    return;
  }
  release(&shared->lock);
  acquire(&segments_lock);
  lst_remove((struct list *)shared);

  Pages_list *list = (Pages_list *)&shared->pages;
  while (!lst_empty(list)) {
    struct shared_page *page = (struct shared_page *)lst_pop(list);
    kfree((void *)page->pa);
    bd_free(page);
  }
  bd_free(shared);
  release(&segments_lock);
}


static void
sharedrefinc(struct shared_segment* seg){
  acquire(&seg->lock);
  seg->refs++;
  release(&seg->lock);
}

// Looks up shared segment with _id_
// returns pointer of finding
// returns 0 on fail
struct shared_segment*
sharedlookup(int id){

  acquire(&segments_lock);

  SharedSegs_list *list = (SharedSegs_list *)&segments;
  struct shared_segment *seg = (struct shared_segment *)list->next;
  for (; (SharedSegs_list *)seg != list;
       seg = (struct shared_segment *)seg->node.next) {
    if (seg->id == id) {
      sharedrefinc(seg);
      release(&segments_lock);
      return seg;
    }
  }
  release(&segments_lock);
  return 0;
}
