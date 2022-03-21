#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//#include "user/user.h"
//extern int trace(int mask, char *argv[]);

uint64
sys_trace(void)
{
  int mask;
  if(argint(0, &mask) < 0){
    return -1;
  }
  myproc()->tracemask = mask;
  return 0;

//   int mask; char *argv[MAXARG];
//   int i;
//   uint64 uargv, uarg;
//   printf("please specify the masssssssk!\n");

//   if(argint(0, &mask) < 0 || argaddr(1, &uargv) < 0){
//     return -1;
//   }
//   memset(argv, 0, sizeof(argv));
//   for(i=0;; i++){
//     if(i >= NELEM(argv)){
//       goto bad;
//     }
//     if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
//       goto bad;
//     }
//     if(uarg == 0){
//       argv[i] = 0;
//       break;
//     }
//     argv[i] = kalloc();
//     if(argv[i] == 0)
//       goto bad;
//     if(fetchstr(uarg, argv[i], PGSIZE) < 0)
//       goto bad;
//   }

//   myproc()->tracemask = mask;
  
//   if (fork() == 0)
//   {
//     exec();
  
//   }
  

//   //int ret = trace(mask, argv);
//   int ret = 55;

//   for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
//     kfree(argv[i]);

//   return ret;

//  bad:
//   for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
//     kfree(argv[i]);
//   return -1;

}

uint64
sys_sysinfo(void)
{
  uint64 addr;
  if (argaddr(0, &addr) < 0)
    return -1;
  struct proc *p = myproc();
  struct sysinfo s;
  //printf("sysinfo the masssssssk111111111!\n");
  s.freemem = kfreemem();
  //printf("sysinfo the masssssssk222222222!\n");
  s.nproc = getnproc();
  //printf("sysinfo the masssssssk3333333333!\n");
  if (copyout(p->pagetable, addr, (char *)&s, sizeof(s)) < 0)
    return -1;
  //printf("sysinfo the masssssssk44444444444!\n");
  //printf("sysinfo the masssssssk!\n");
  return 0;
  
}
