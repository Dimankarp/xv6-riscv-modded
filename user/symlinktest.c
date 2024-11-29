#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

static void cmp_file_contents(char *filename, char *buf, int len,
                              char *cmp_str) {
  int symfd;
  int rbytes;
  if ((symfd = open(filename, O_RDWR)) < 0) {
    printf("symlinktest:[FAIL]: failed to open symlink\n");
    exit(1);
  }
  printf("symlinktest: opened symlink\n");

  if ((rbytes = read(symfd, buf, len)) != len) {
    printf(
        "symlinktest:[FAIL]: failed to read %d bytes from test file %s, only "
        "read %d\n",
        len, filename, rbytes);
    exit(1);
  }
  if (strcmp(cmp_str, buf)) {
    printf("symlinktest:[FAIL]: strings are not equal\n");
    printf("symlinktest:[FAIL]: test string: %s\n", cmp_str);
    printf("symlinktest:[FAIL]: read string: %s\n", buf);
    exit(1);
  }
}

void test_file() {
  char *string =
      "So why is it called mksym() instead of symlink()? - If only I knew...";
  char buf[256] = {0};
  int len = strlen(string);
  int wbytes;

  printf("symlinktest:[TEST FILE]: start\n");
  printf("symlinktest: removing symlinktest.test file\n");
  unlink("symlinktest.test");
  unlink("symlinktest.link");
  int fd = open("symlinktest.test", O_CREATE | O_TRUNC | O_RDWR);

  if ((wbytes = write(fd, string, len)) < len) {
    printf("symlinktest:[FAIL]: failed to write %d bytes to test file, only "
           "wrote %d\n",
           len, wbytes);
    exit(1);
  }
  printf("symlinktest: wrote test string\n");

  if ((mksym("symlinktest.text", "symlinktest.link", 1)) != 0) {
    printf("symlinktest:[FAIL]: failed to create symlink\n");
    exit(1);
  }
  printf("symlinktest: created symlink\n");

  cmp_file_contents("symlinktest.link", buf, len, string);

  printf("symlinktest:[SUCCES]: write->read test passed\n");
}

void test_single_dir() {

  char *string = "I wish I was a dentist, like my mother wanted...";
  char buf[256] = {0};
  int len = strlen(string);
  int wbytes;
  printf("symlinktest:[TEST DIR]: start\n");
  printf("symlinktest: removing testsym.dir/symlinktest.test file, sym.link & "
         "sym.dir\n");

  unlink("testsym.dir/symlinktest.test");
  unlink("testsym.dir");
  unlink("sym.link");
  unlink("sym.dir");

  printf("symlinktest: creating test file\n");

  mkdir("testsym.dir");
  int fd = open("testsym.dir/symlinktest.test", O_CREATE | O_TRUNC | O_RDWR);

  if ((wbytes = write(fd, string, len)) < len) {
    printf("symlinktest:[FAIL]: failed to write %d bytes to test file, only "
           "wrote %d\n",
           len, wbytes);
    exit(1);
  }
  printf("symlinktest: Testing linking through dir");
  mksym("testsym.dir/symlinktest.test", "sym.link", 0);
  cmp_file_contents("sym.link", buf, len, string);

  printf("symlinktest: Testing dir substitue");
  mksym("testsym.dir", "sym.dir", 0);
  cmp_file_contents("sym.dir/symlinktest.test", buf, len, string);

  printf("symlinktest:[SUCCES]: single dir test passed\n");
}

void test_multiple_dir() {

  char *string =
      "Don't even try to do recursive symlinks...I haven't tested them...";
  char buf[256] = {0};
  int len = strlen(string);
  int wbytes;
  printf("symlinktest:[TEST DIR]: start\n");
  printf("symlinktest: removing test files\n");

  unlink("test.dir1/test.dir2/test.dir3/test");
  unlink("test.dir1/test.dir2/test.dir3");
  unlink("test.dir1/test.dir2");
  unlink("test.dir1");
  unlink("sym.dir");

  // printf("symlinktest: creating test file\n");

  mkdir("test.dir1");
  // ;
  mkdir("test.dir1/test.dir2");
  mkdir("test.dir1/test.dir2/test.dir3");
  int fd =
      open("test.dir1/test.dir2/test.dir3/test", O_CREATE | O_TRUNC | O_RDWR);
  if ((wbytes = write(fd, string, len)) < len) {
    printf("symlinktest:[FAIL]: failed to write %d bytes to test file, only "
           "wrote %d\n",
           len, wbytes);
    exit(1);
  }
  printf("symlinktest: Testing multiple dir substitue\n");
  mksym("test.dir1/test.dir2/test.dir3", "sym.dir", 0);
  cmp_file_contents("sym.dir/test", buf, len, string);

  printf("symlinktest:[SUCCES]: multiple dir test passed\n");
}

void test_rm() {
  char *string = "Where does deleted files go? Can we get back that Skyrim "
                 "savegame from when I was 15?";
  char buf[256] = {0};
  int len = strlen(string);
  int wbytes;
  printf("symlinktest:[TEST DIR]: start\n");
  printf("symlinktest: removing test files\n");

  unlink("symlink.test");
  unlink("sym.link");

  printf("symlinktest: creating test file\n");

  int fd = open("symlink.test", O_CREATE | O_TRUNC | O_RDWR);

  if ((wbytes = write(fd, string, len)) < len) {
    printf("symlinktest:[FAIL]: failed to write %d bytes to test file, only "
           "wrote %d\n",
           len, wbytes);
    exit(1);
  }

  printf("symlinktest: Creating symlink for test file\n");
  mksym("symlink.test", "sym.link", 0);
  cmp_file_contents("sym.link", buf, len, string);

  printf("symlinktest: Removing test file\n");
  if (unlink("symlink.test") < 0) {
    printf("symlinktest:[FAIL] failed to remove test file");
    exit(1);
  };
  printf("symlinktest: Expect to fail open symlink\n");
  fd = open("sym.link", O_CREATE | O_TRUNC | O_RDWR);
  if (fd != -1) {
    printf("symlinktest:[FAIL] successfully opened symlink that refers to "
           "removed file");
    exit(1);
  }
  printf("symlinktest:[SUCCES]: fail to open after remove test passed\n");
}

void test_symlink_chain() {
  char *string = "Where does deleted files go? Can we get back that Skyrim "
                 "savegame from when I was 15?";
  char buf[256] = {0};
  int len = strlen(string);
  int wbytes;
  printf("symlinktest:[TEST DIR]: start\n");
  printf("symlinktest: removing test files\n");

  unlink("symlink.dir/symlink.test");
  unlink("symlink.dir");
  unlink("sym.link1");
  unlink("sym.link2");
  unlink("sym.link3");

  printf("symlinktest: creating test file\n");

  mkdir("symlink.dir");
  int fd = open("symlink.dir/symlink.test", O_CREATE | O_TRUNC | O_RDWR);

  if ((wbytes = write(fd, string, len)) < len) {
    printf("symlinktest:[FAIL]: failed to write %d bytes to test file, only "
           "wrote %d\n",
           len, wbytes);
    exit(1);
  }

  printf("symlinktest: Creating symlink chain\n");
  mksym("symlink.dir", "sym.link1", 0);
  mksym("sym.link1", "sym.link2", 0);
  mksym("sym.link2", "sym.link3", 0);

  printf("symlinktest: trying to open through chain\n");
  cmp_file_contents("sym.link3/symlink.test", buf, len, string);
  printf("symlinktest:[SUCCES]: symlink chain test passed\n");
}
int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  test_file();
  test_single_dir();
  test_multiple_dir();
  test_rm();
  test_symlink_chain();
  exit(0);
}