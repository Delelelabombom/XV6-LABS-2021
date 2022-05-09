// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end, int cid);
void kfreewithcid(void *pa, int cid);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct cmem
{
  struct spinlock lock;
  struct run *freelist;
} ;

struct cmem cmemlist[NCPU] ;

void
kinit()
{
  // initlock(&kmem.lock, "kmem");
  // freerange(end, (void*)PHYSTOP);
  int gap = 128 * 1024 * 1024 / NCPU;
  void *laddr = (void*)end;
  void *raddr = (void*)(KERNBASE + gap);
  for (int i = 0; i < NCPU; i++){
    initlock(&(cmemlist->lock), "kmem");
    freerange(laddr, raddr, i);
    laddr = raddr;
    raddr += gap;
  }
  
}

void
freerange(void *pa_start, void *pa_end, int cid)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfreewithcid((void *)p, cid);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfreewithcid(void *pa, int cid)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&cmemlist[cid].lock);
  r->next = cmemlist[cid].freelist;
  cmemlist[cid].freelist = r;
  release(&cmemlist[cid].lock);
}

void
kfree(void *pa)
{
  push_off();
  int cid = cpuid();
  pop_off();
  kfreewithcid(pa, cid);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int cid = cpuid(); 
  pop_off();
  acquire(&cmemlist[cid].lock);
  r = cmemlist[cid].freelist;
  if(r)
    cmemlist[cid].freelist = r->next;
  else{
    for (int i = 0; i < NCPU; i++)
    {
      if (i == cid)
        continue;
      struct run *r1;
      acquire(&cmemlist[i].lock);
      r1 = cmemlist[i].freelist;
      if (r1)
      {
        cmemlist[i].freelist = r1->next;
        r = r1;
        release(&cmemlist[i].lock);
        break;
      }
      release(&cmemlist[i].lock);
    }
  }
  release(&cmemlist[cid].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
