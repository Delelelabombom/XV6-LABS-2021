#include "kernel/types.h"
#include "kernel/syscall.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
  //char *nargv[MAXARG];

  if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')){
    fprintf(2, "Usage: %s mask command\n", argv[0]);
    exit(1);
  }

  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }

  int i;
  char *argvs[MAXARG];
  for (i = 2; i < argc; i++)
  {
    argvs[i - 2] = argv[i]; 
  }
  exec(argvs[0], argvs);
  // if (fork() == 0)
  // {
  //   //printf("!!!%s----\n", execpath);
    
    
  // }
  
  exit(0);


  // int i;
  // char *nargv[MAXARG];

  // if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')){
  //   fprintf(2, "Usage: %s mask command\n", argv[0]);
  //   exit(1);
  // }

  // // 只需要实现对argv[1]的记录就好
  // if (trace(atoi(argv[1])) < 0) {
  //   fprintf(2, "%s: trace failed\n", argv[0]);
  //   exit(1);
  // }
  
  // for(i = 2; i < argc && i < MAXARG; i++){
  //   nargv[i-2] = argv[i];
  // }
  // exec(nargv[0], nargv);
  // exit(0);
}