// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem {
  struct spinlock lock;
  struct run *freelist;
} kmems[NCPU];

void
kinit()
{
  static char digits[] = "0123456789";
  static char buf[] = "kmem?";
  for (int i = 0; i < NCPU; ++i) {
    buf[4] = digits[i];
    initlock(&kmems[i].lock, buf);
  }
  freerange(end, (void*)PHYSTOP);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfreehelper(void *pa, struct kmem* kmem)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem->lock);
  r->next = kmem->freelist;
  kmem->freelist = r;
  release(&kmem->lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kallochelper(struct kmem* kmem)
{
  struct run *r;

  acquire(&kmem->lock);
  r = kmem->freelist;
  if(r)
    kmem->freelist = r->next;
  release(&kmem->lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void kfree(void *pa) {
  push_off();
  int i = cpuid();
  kfreehelper(pa, &kmems[i]);
  pop_off();
}

void* kalloc(void) {
  push_off();
  int i = cpuid();
  void* r = kallochelper(&kmems[i]);
  pop_off();
  if (!r) for (int j = 1; j < 8; ++j) {
    r = kallochelper(&kmems[(i+j)%NCPU]);
    if (r) break;
  }
  return r;
}


void
freerange(void *pa_start, void *pa_end)
{
  push_off();
  int id = cpuid();
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfreehelper(p, &kmems[id]);
  pop_off();
}
