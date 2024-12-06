struct proc_queue {
struct list queue;
struct spinlock q_lock;
};

struct feedback_row{
    uint16 quantums;
    uint16 tqexp;
    uint16 slpret;
};
