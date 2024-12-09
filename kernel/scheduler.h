struct proc_queue {
  struct list queue;
  struct spinlock q_lock;
};
struct proc_entry {
  struct list node;
  struct proc* p;
};

struct feedback_row {
  uint16 quantums;
  uint16 tqexp;
  uint16 slpret;
};
