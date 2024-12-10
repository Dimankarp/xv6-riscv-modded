#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

#define PROC_NUM 120
void manyforks() {
  int pids[PROC_NUM];
  int i = 0;
  int pid = 1;
  while (pid != 0 && i < PROC_NUM) {
    pid = fork();
    if (pid < 0) {
      printf("fork failed at i : %d", i);
      exit(1);
    }
    if (pid == 0) {
      for (;;)
        ;
    }
    pids[i++] = pid;
  }
  for(int j = 0; j < i; j++){
    kill(pids[j]);
  }
    for(int j = 0; j < i; j++){
    wait(0);
  }
  return;
}

int main() {
  printf("[manyforks]: testing forks of %d procs\n", PROC_NUM);
  manyforks();
  printf("[manyforks]: test passed\n");
  exit(0);
}