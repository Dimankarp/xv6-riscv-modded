#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 3 && argc != 4) {
    fprintf(2, "Usage: ln [-s] old new\n");
    exit(1);
  }
  if (argc == 4) {
    if (strcmp(argv[1], "-s") != 0) {
      fprintf(2, "Wrong key. Only [-s] is supported.\n");
      exit(1);
    }
    if (mksym(argv[2], argv[3], 0) < 0) {
      fprintf(2, "symlink %s %s: failed\n", argv[2], argv[3]);
      exit(1);
    }
    exit(0);
  } else if (link(argv[1], argv[2]) < 0)
    fprintf(2, "link %s %s: failed\n", argv[1], argv[2]);
  exit(0);
}
