/*
 * memdebug.c
 *
 *  Created on: 15 Dec 2010
 *      Author: Andy Brown
 *
 *  Use without attribution is permitted provided that this
 *  header remains intact and that these terms and conditions
 *  are followed:
 *
 *  http://andybrown.me.uk/ws/terms-and-conditions
 */
 
#ifdef DEBUG
#ifdef __AVR__
 
#include <stdlib.h>
#include <avr/io.h>
#include "memdebug.h"


/**
 * This must match the definition in "stdlib_private.h"
 */
 
typedef struct __freelist {
  size_t sz;
  struct __freelist *nx;
} FREELIST;
 
extern FREELIST *__flp;
extern char *__brkval;
 
 
/**
 * Get the total memory used by your program. The total will
 * include accounting overhead internal to the library
 */
 
size_t getMemoryUsed()
{
  size_t used;
  FREELIST *fp;
 
// __brkval=0 if nothing has been allocated yet
 
  if(__brkval==0)
    return 0;
 
// __brkval moves up from __malloc_heap_start to
// __malloc_heap_end as memory is used
 
  used=__brkval-__malloc_heap_start;
 
// memory free'd by you is collected in the free list and
// compacted with adjacent blocks. This, combined with malloc's
// intelligent picking of candidate blocks drastically reduces
// heap fragmentation. Anyway, since blocks in the free list
// are available to you at no cost we need to take them off.
 
  for(fp=__flp;fp;fp=fp->nx)
    used-=fp->sz+sizeof(size_t);
 
  return used;
}
 
 
/**
 * Get the total free bytes
 */
 
size_t getFreeMemory()
{
  return (size_t)AVR_STACK_POINTER_REG-
         (size_t)__malloc_margin-
         (size_t)__malloc_heap_start-
         getMemoryUsed();
}
 
 
/**
 * Get the largest available block that can be successfully
 * allocated by malloc()
 */
 
size_t getLargestAvailableMemoryBlock()
{
  size_t a,b;
 
  a=getLargestBlockInFreeList();
  b=getLargestNonFreeListBlock();
 
  return a>b ? a : b;
}
 
 
/**
 * Get the largest block in the free list
 */
 
size_t getLargestBlockInFreeList()
{
  FREELIST *fp;
  size_t maxsize=0;
 
  for(fp=__flp;fp;fp=fp->nx)
    if(fp->sz>maxsize)
      maxsize=fp->sz;
 
  return maxsize;
}
 
 
/**
 * Get the number of blocks in the free list
 */
 
int getNumberOfBlocksInFreeList()
{
  FREELIST *fp;
  int i;
 
  for(i=0,fp=__flp;fp;fp=fp->nx,i++);
  return i;
}
 
 
/**
 * Get total size of free list (includes library overhead)
 */
 
size_t getFreeListSize()
{
  FREELIST *fp;
  size_t size;
 
  for(size=0,fp=__flp;fp;fp=fp->nx,size+=fp->sz+sizeof(size_t));
  return size;
}
 
 
/**
 * Get the largest block that can be successfully allocated
 * without reuse from the free list
 */
 
size_t getLargestNonFreeListBlock()
{
  char *cp,*brkval;
 
// this code is an adapted fragment from malloc() itself
 
  brkval=__brkval == 0 ? __malloc_heap_start : __brkval;
 
  if((cp=__malloc_heap_end)==NULL)
    cp=(char *)AVR_STACK_POINTER_REG-__malloc_margin;
  if(cp<=brkval)
    return 0;
 
  return cp-brkval;
}
 
#endif //__AVR__
#endif // DEBUG
