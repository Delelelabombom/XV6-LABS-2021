#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int clock_ticks;

  if(argc <= 1){
    fprintf(2, "please specify the clock ticks!\n");
    exit(1);
  }
  clock_ticks = atoi(argv[1]);
  sleep(clock_ticks);
  exit(0);
}