#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"
uint64 acquire_freemem();
uint64 acquire_nproc();
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

int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
    uint64 addr;
    int len;
    int bitmask;
    if(argaddr(0,&addr)<0) return -1;
    if(argint(1,&len)<0) return -1;
    if(argint(2,&bitmask)<0) return -1;
    
    if(len>32||len<0) return -1;
    int res = 0;

    struct proc *p = myproc();
    for(int i=0;i<len;i++){
      int va = addr + i*PGSIZE;
      int abits = vm_pgaccess(p->pagetable,va);
      res = res | abits<<i;
    }
    if(copyout(p->pagetable,bitmask,(char*)&res,sizeof(res))<0) return -1;
    return 0;
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

uint 
sys_trace(void)
{
  int mask;
  if(argint(0, &mask) < 0)
    return -1;
  struct proc *p = myproc();
  p->trace_mask = mask;
  return 0;
}
uint64 
sys_sysinfo(void)
{
  struct sysinfo info;
  uint64 addr;
  struct proc *p = myproc();
  info.nproc = acquire_nproc();
  info.freemem = acquire_freemem();
  if(argaddr(0, &addr) < 0)
    return -1;
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  printf("sysinfo say hi\n");
  return 0;
}