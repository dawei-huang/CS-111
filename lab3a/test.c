#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <fcntl.h>
#include<time.h>
#include<sys/time.h>
#include "ext2_fs.h"

int main() {
  fprintf(stderr, "size of: %d\n", (int) sizeof(struct ext2_super_block));
  fprintf(stderr, "size of: %d\n", (int) sizeof(struct ext2_group_desc));

  return 0;
}
