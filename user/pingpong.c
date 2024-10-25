#include "kernel/types.h"
#include "user/user.h"

int 
main(int argc, char *argv[]) {
  /*
  Since wait() is not among the
  recommended syscalls for this task
   - creating 2 pairs of pipes
  */
  int ppw[2]; // pipes parent write
  int psw[2]; // pipes son write
  pipe(ppw);
  pipe(psw);
  char buf[5] = {0};

  if (fork() == 0) {
    read(ppw[0], buf, 5);
    printf("%d: got %s\n", getpid(), buf);
    write(psw[1], "pong", 5);
  } else {
    write(ppw[1], "ping", 5);
    read(psw[0], buf, 5);
    printf("%d: got %s\n", getpid(), buf);
  }

  exit(0);
}
