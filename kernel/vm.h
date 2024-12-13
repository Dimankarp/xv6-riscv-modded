// PTE is blocked when it's shared by
// multiple procs as a result of forking.
// It's unblocked on page fault.
#define PTE_BLCKD PTE_RSWL