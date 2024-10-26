#include "kernel/types.h"
#include "user/user.h"

#define EXIT_ON_ERR(status)                                                    \
  do {                                                                         \
    if (status == -1) {                                                        \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

int 
main(int argc, char *argv[]) {
  /*
  Since wait() is not among the
  recommended syscalls for this task
   - creating 2 pairs of pipes
  */
  int ppw[2]; // pipes parent write
  int psw[2]; // pipes son write
  EXIT_ON_ERR(pipe(ppw));
  EXIT_ON_ERR(pipe(psw));

  char buf[5] = {0};
  int pid = fork();
  EXIT_ON_ERR(pid);

  if (pid == 0) {
    EXIT_ON_ERR(read(ppw[0], buf, 5));
    printf("%d: got %s\n", getpid(), buf);
    EXIT_ON_ERR(write(psw[1], "pong", 5));
  } else {
    EXIT_ON_ERR(write(ppw[1], "ping", 5));
    EXIT_ON_ERR(read(psw[0], buf, 5));
    printf("%d: got %s\n", getpid(), buf);
  }

  exit(0);
}
