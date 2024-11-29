// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};


// Recursive mutual exclusion lock.
struct recursive_spinlock {
  uint locked;       // Is the lock held?
  uint count;        // Times taken by cpu
  struct cpu *cpu;   // The cpu holding the lock.
  // For debugging:
  char *name;        // Name of lock.

};

