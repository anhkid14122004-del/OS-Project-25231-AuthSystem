#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("Executing ps syscall...\n");
  ps();
  exit(0);
}
