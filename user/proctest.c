#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
preempt(char *s)
{
  char buf[255];
  int pid1, pid2, pid3, pid4;
  int pfds[2];

  pid1 = fork();
  if(pid1 < 0) {
    printf("%s: fork failed", s);
    exit(1);
  }
  if(pid1 == 0){
    for(;;)
      ;
  }
  pid2 = fork();
  if(pid2 < 0) {
    printf("%s: fork failed\n", s);
    exit(1);
  }
  if(pid2 == 0){
        for(;;)
      ;
  }

    pid4 = fork();
  if(pid4 < 0) {
    printf("%s: fork failed\n", s);
    exit(1);
  }
  if(pid4 == 0){
        for(;;)
      ;
  }


  pipe(pfds);
  pid3 = fork();
  if(pid3 < 0) {
     printf("%s: fork failed\n", s);
     exit(1);
  }
  if(pid3 == 0){
    close(pfds[0]);
    if(write(pfds[1], "x", 1) != 1)
      printf("%s: preempt write error", s);
    close(pfds[1]);
    printf("WROTE\n");
    for(;;)
      ;
  }
  printf("closing\n");
  close(pfds[1]);
    printf("reading\n");
  if(read(pfds[0], buf, sizeof(buf)) != 1){
    printf("%s: preempt read error", s);
    return;
  }
  printf("done");
  close(pfds[0]);
  printf("kill... ");
  kill(pid1);
  kill(pid2);
  kill(pid3);
    kill(pid4);
  printf("wait... ");
  wait(0);
  wait(0);
  wait(0);
  wait(0);
}

int main(){
    preempt("preempt");
    exit(0);
}