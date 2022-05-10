// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13

struct {
  struct spinlock biglock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[NBUCKET];
  struct spinlock locks[NBUCKET];

} bcache;

uint
hash(uint i){
  return i % NBUCKET;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.biglock, "bcache");

  for (int i = 0; i < NBUCKET; i++)
  {
    initlock(&(bcache.locks[i]), "bcache");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }
  

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  uint indexBucket = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[indexBucket].next;
    b->prev = &bcache.head[indexBucket];
    initsleeplock(&b->lock, "buffer");
    bcache.head[indexBucket].next->prev = b;
    bcache.head[indexBucket].next = b;
    indexBucket = hash(indexBucket + 1);
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  uint locksIndex = hash(blockno);
  acquire(&bcache.locks[locksIndex]);

  // Is the block already cached?
  for(b = bcache.head[locksIndex].next; b != &bcache.head[locksIndex]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->lastUse = ticks;
      release(&bcache.locks[locksIndex]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.locks[locksIndex]);
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.

  //acquire(&bcache.locks[locksIndex]);
  // for(b = bcache.head[locksIndex].prev; b != &bcache.head[locksIndex]; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0;
  //     b->refcnt = 1;
  //     b->lastUse = ticks;
  //     release(&bcache.locks[locksIndex]);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // } 

  // 
  
  uint bucketIndex = locksIndex;
  int j = 0;
  acquire(&bcache.biglock);

  //这里需要重新找一遍
  acquire(&bcache.locks[locksIndex]);

  // Is the block already cached?
  for(b = bcache.head[locksIndex].next; b != &bcache.head[locksIndex]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->lastUse = ticks;
      release(&bcache.locks[locksIndex]);
      release(&bcache.biglock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.locks[locksIndex]);

  for (; j < NBUCKET; j++){
    
    acquire(&bcache.locks[bucketIndex]);
    for(b = bcache.head[bucketIndex].prev; b != &bcache.head[bucketIndex]; b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        b->lastUse = ticks;
        if (bucketIndex != locksIndex){
          b->prev->next = b->next;
          b->next->prev = b->prev;
          
          release(&bcache.locks[bucketIndex]);
          acquire(&bcache.locks[locksIndex]);
          
          b->next = bcache.head[locksIndex].next;
          b->prev = &bcache.head[locksIndex];
          bcache.head[locksIndex].next->prev = b;
          bcache.head[locksIndex].next = b;
          release(&bcache.locks[locksIndex]);
          release(&bcache.biglock);
          
          acquiresleep(&b->lock);
          return b;
        }
        release(&bcache.locks[bucketIndex]);
        release(&bcache.biglock);
        
        acquiresleep(&b->lock);
        return b;
      }
    } 
    release(&bcache.locks[bucketIndex]);
    
    bucketIndex = hash(bucketIndex + 1);
  }
  release(&bcache.biglock);
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  //printf("bwirte %d\n", b->blockno);
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  uint locksindex = hash(b->blockno);
  releasesleep(&b->lock);

  acquire(&bcache.locks[locksindex]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[locksindex].next;
    b->prev = &bcache.head[locksindex];
    bcache.head[locksindex].next->prev = b;
    bcache.head[locksindex].next = b;
  }
  
  release(&bcache.locks[locksindex]);
}

void
bpin(struct buf *b) {
  uint locksindex = hash(b->blockno);
  acquire(&bcache.locks[locksindex]);
  b->refcnt++;
  release(&bcache.locks[locksindex]);
}

void
bunpin(struct buf *b) {
  uint locksindex = hash(b->blockno);
  acquire(&bcache.locks[locksindex]);
  b->refcnt--;
  release(&bcache.locks[locksindex]);
}


