#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

static void fail(char *reason) {
  printf("sharedtest:[FAIL]: %s\n", reason);
  exit(1);
}

void test_page() {

  printf("sharedtest:[TEST PAGE]: start\n");
  printf("sharedtest: sbrk() space for a page\n");

  // mapping 2 pages, since our brk may be at the middle
  // of another page
  uint64 addr = (uint64)sbrk(2 * 4096);

  if (addr == -1)
    fail("failed to sbrk");

  printf("sharedtest: shmget()\n");
  int id;
  int sharedfd = shmget(4096, &id);
  if (sharedfd == -1)
    fail("failed to shmget");
  printf("sharedtest: sharedfd %d\n", sharedfd);
  printf("sharedtest: sharedid %d\n", id);
  printf("sharedtest: mapping shared");
  void *sharedadr;
  if ((sharedadr = shmmap(sharedfd, (void *)addr)) == 0)
    fail("failed to shmmap");
  printf("sharedtest: mapped to %p\n", sharedadr);

  printf("sharedtest: write check\n");
  *(char *)sharedadr = 'B';

  printf("sharedtest: forking\n");
  int pid = fork();
  if (pid == 0) {
    printf("sharedtest[child]: looking up\n");
    int newfd = shmlookup(id);
    if (newfd == -1)
      fail("failed to lookup");
    if ((sharedadr = shmmap(newfd, (void *)addr)) == 0)
      fail("failed to shmmap");
    if (*(char *)sharedadr != 'B')
      fail("didn't read B from shared memory");
    printf("sharedtest[child]: successfully read B\n");
    exit(0);
  }

  int status;
  wait(&status);
  if (status != 0)
    fail("child exit status is not zero");
  printf("sharedtest:[TEST PAGE] OK \n");
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  test_page();
  exit(1);
}