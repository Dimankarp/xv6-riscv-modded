#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void test_file()
{
  char* string = "So why is it called mksym() instead of symlink()? - If only I knew...";
  char buf[256] = {0};
  int len = strlen(string);
  int wbytes;
  int rbytes;

  printf("symlinktest:[TEST FILE]: start\n");
  printf("symlinktest: removing symlinktest.test file\n");
  unlink("symlinktest.test");
  unlink("symlinktest.link");
  int fd = open("symlinktest.test", O_CREATE|O_TRUNC|O_RDWR);

  if((wbytes = write(fd, string, len)) < len){
    printf("symlinktest:[FAIL]: failed to write %d bytes to test file, only wrote %d\n", len ,wbytes);
    exit(1);
  }
  printf("symlinktest: wrote test string\n");

  if((mksym("symlinktest.text", "symlinktest.link", 1)) != 0){
    printf("symlinktest:[FAIL]: failed to create symlink\n");
    exit(1);
  }
   printf("symlinktest: created symlink\n");

    int symfd;
   if((symfd = open("symlinktest.link", O_RDWR)) < 0){
    printf("symlinktest:[FAIL]: failed to open symlink\n");
    exit(1);
   }
  printf("symlinktest: opened symlink\n");

  if((rbytes = read(symfd, buf, len)) != len){
    printf("symlinktest:[FAIL]: failed to read %d bytes to test file, only read %d\n", len ,rbytes);
    exit(1);
  }
  if(strcmp(string, buf)){
    printf("symlinktest:[FAIL]: strings are not equal\n");
    printf("symlinktest:[FAIL]: test string: %s\n", string);
    printf("symlinktest:[FAIL]: read string: %s\n", buf);
    exit(1);
  }
  printf("symlinktest:[SUCCES]: write->read test passed\n");
}

void test_dir(){

    // int fd;
    printf("symlinktest:[TEST DIR]: start\n");
    printf("symlinktest: removing symlinktest.test file\n");
    unlink("symlinktest.dir/symlinktest.test");
    unlink("symlinktest.dir");
    mkdir("symlinktest.dir");

    // if((fd = open("symlinktest.dir/symlinktest.test", O_CREATE|O_TRUNC|O_RDWR)) < 0){
    //     printf("symlinktest:[FAIL]:")
    // }
}


int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  test_file();
  exit(0);
}