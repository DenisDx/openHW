/*
 * memdebug.h
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
 
#ifdef __cplusplus
extern "C" {
#endif
 
 
#include <stddef.h>

 
extern size_t getMemoryUsed();
extern size_t getFreeMemory();
extern size_t getLargestAvailableMemoryBlock();
extern size_t getLargestBlockInFreeList();
extern int getNumberOfBlocksInFreeList();
extern size_t getFreeListSize();
extern size_t getLargestNonFreeListBlock();
 
 
#ifdef __cplusplus
}
#endif
 
#endif //__AVR__
#endif // DEBUG
