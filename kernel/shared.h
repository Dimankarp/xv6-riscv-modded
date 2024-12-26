struct shared_page{
  struct list node;
  uint64 pa;
};

struct shared_segment {
  struct list node;
  struct spinlock lock;
  int id;
  uint8 refs;
  uint16 pages_count;
  Pages_list pages;
};