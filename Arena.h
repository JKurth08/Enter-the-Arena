#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

typedef int8_t       i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t      u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef u8  b8;

#define ARENA_ALIGN     sizeof(void*)  /* defualt alignment */
_Static_assert((ARENA_ALIGN & (ARENA_ALIGN - 1)) == 0, "ARENA_ALIGN must be a power of 2");

#define ALIGN_UP(n, p)  (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))

#define KiB(n)          ((u64)(n) << 10)
#define MiB(n)          ((u64)(n) << 20)
#define GiB(n)          ((u64)(n) << 30)

typedef struct arena{
    u64 capacity;
    u64 pos;

    u8* mem_region;

    struct arena* next;

    #ifdef DEBUG
    u64 num_of_allocations;
    #endif  /*DEBUG*/

} memory_arena;


////////    Public  ////////       
////////     API    ////////

/*
Function serves as the setup for the struct. 
If any paramater is NULL or missing, it will 
return. Function does not malloc any 
memory so it will naturally use the stack.

memory_arena *arena:    The arena struct you wish to init
u64 capacity:           The total number of bytes you wish to allocate to a single memory region
void* mem_region:       The region of memory reserved for allocations
*/
void    arenaInit(memory_arena *arena, u64 capacity, void* mem_region);

/*
This function creates an arena with a desired 
capacity (total number of bytes). Returns a pointer 
to the start of the memory region struct that has been allocated. 
This arena is chain aware: if allocations go over capacity, 
a new memory region will be created and the current arena
will be linked to the new region and so on.
Function returns NULL if capacity not provided or 0.

u64 capacity:   The total number of bytes you wish to allocate to a single memory region
*/
void*   arenaCreate(u64 capacity);

/*
Function destroys the arena by cleaning out it contents and 
freeing the memory region(s) and the struct itself.

memory_arena* arena:    The arena you wish to destory
*/
void    arenaDestroy(memory_arena* arena);

/*
Function resets the arena (and any chained arenas) to
their initial state without freeing memory so all
blocks stay available for reuse.

memory_arena* arena:    The arena you wish to clear
*/
void    arenaClear(memory_arena* arena);

/*
Function allocates the desired size (in bytes) into the 
memory region. Bumps the position forward and returns a 
pointer to the start of the allocation. Allocations are 
aligned to the size of a void* on your system. If you 
need to allocate more memory than intially requested, 
you can. The arena will link another arena object with 
the same capacity reserved for the memory region. Allocations 
will continue being accepted. Be mindful of your memory, 
many arena objects is less efficient. Will return NULL if: 
size > capacity, no size or arena is provided, size is 0.

memory_arena* arena:    A pointer to the arena you wish to alloc into
u64 size:               The size (in bytes) that you wish to allocate
*/
void*   arena_alloc(memory_arena* arena, u64 size);

// Available under -DDEBUG
#ifdef DEBUG

/*
Function prints formatted: Arena # (position in list),
how many allocations were made respective to the arena,
capacity, pos, and the address's of the memory region reserved.
Turn flag printCurrent on or off to either print the current arena 
(last in the list) or all of them.
*/
void printArenaInfo(memory_arena* arena, b8 printCurrent);

#endif  /* DEBUG */

#endif  /* ARENA_H */
