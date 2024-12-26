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

void set_and_wait_change(char *addr, char sym, char waitfor) {
  *addr = sym;
  while (*addr != waitfor)
  // Tests go in the infinite cycle
  // without fences.
      __sync_synchronize();
}

void wait_change_and_set(char *addr, char sym, char waitfor) {
  while (*addr != waitfor)
      __sync_synchronize();
  *addr = sym;
}

void test_producer() {
  uint64 addr = (uint64)sbrk(11 * 4096);
  if (addr == -1)
    exit(1);

  int id;
  int sharedfd = shmget(10 * 4096, &id);
  if (sharedfd == -1)
    exit(1);

  char* sharedadr;
  if ((sharedadr = shmmap(sharedfd, (char *)addr)) == 0)
    exit(1);
  memset(sharedadr, 0, 10  * 4096);
  //Exchanging id with consumer
  char idc = id;
  write(1, &idc, 1);

  for (int i = 0; i < 10 * 4096; ++i) {
    
    set_and_wait_change(sharedadr, 'P', 'B');
    sharedadr++;
  }
}

void test_consumer() {
 uint64 addr = (uint64)sbrk(11 * 4096);
  if (addr == -1)
    exit(1);


  char id = -1;
  // Apparently there is no scanf...
  printf("Consumer reading:\n");
  read(0, &id, 1);
  printf("Consumer read %d\n", id);
  int sharedfd = shmlookup(id);
  if (sharedfd == -1)
    fail("Consumer: lookup failed\n");

  char *sharedadr;
  if ((sharedadr = shmmap(sharedfd, (char *)addr)) == 0)
    fail("Consumer: shmmap failed\n");

  for (int i = 0; i < 10 * 4096; ++i) {
    wait_change_and_set(sharedadr, 'B', 'P');
    sharedadr++;
  }

  printf("Successfully ended shared memory exchange!\n");
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  if (argc >= 2) {
    if (strcmp("-p", argv[1]) == 0)
      test_producer();
    else if (strcmp("-c", argv[1]) == 0)
      test_consumer();
    else
      fail("failed to recognize options");
  } else {
    test_page();
  }
  exit(0);
}