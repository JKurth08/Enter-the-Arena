#include "Arena.h"
#include <stdio.h>

void arenaInit(memory_arena *arena, u64 capacity, void* mem_region) {

    if (!arena || !capacity || !mem_region) {
        return;
    }
    
    arena->capacity = capacity;
    arena->pos = 0;
    arena->mem_region = mem_region;

    arena->next = NULL;

    #ifdef DEBUG  
    arena->num_of_allocations = 0;
    #endif   /* DEBUG */

}


void* arenaCreate(u64 capacity) {

    if (!capacity) {
        return NULL;
    }

    memory_arena* arena = (memory_arena*)malloc(sizeof(memory_arena));
    if (!arena) {
        return NULL;    /* if this fails your cooked */
    }

    u8* region = (u8*)malloc(capacity);
    if (!region) {
        free(arena);
        return NULL;
    }

    arenaInit(arena, capacity, region);

    return arena;
}


void arenaDestroy(memory_arena* arena) {

    if (!arena) {
        return;
    }

    memory_arena* current = arena;

    while (current != NULL) {

        memory_arena* next = current->next;
        free(current->mem_region);
        free(current);

        current = next;
    }
}


void arenaClear(memory_arena* arena) {

    if (!arena) {
        return;
    }

    memory_arena* current = arena;

    while (current != NULL) {

        current->pos = 0;

        #ifdef DEBUG
        current->num_of_allocations = 0;    
        #endif   /* DEBUG */

        current = current->next;
    }
}


void* arena_alloc(memory_arena* arena, u64 size) {

    if (!arena || size == 0) {
        return NULL;
    }

    /* a single alloc can never be bigger than capacity */
    if (size > arena->capacity) {
        return NULL;
    }

    memory_arena* current = arena;

    while (current->next != NULL && current->next->pos != 0) {
        current = current->next;
    }


    u8* base = (u8*)current->mem_region + current->pos;
    u8* aligned_base = (u8*)ALIGN_UP((u8*)base, ARENA_ALIGN);
    u64 align_pos = aligned_base - (u8*)current->mem_region; 
    u64 new_pos = align_pos + size;
    

    if (new_pos > current->capacity) {

        if (!current->next) {
            current->next = arenaCreate(arena->capacity);
        }

        u8* next_base = (u8*)current->next->mem_region;
        u8* next_aligned = (u8*)ALIGN_UP((u8*)next_base, ARENA_ALIGN);
        u64 new_pos = (next_aligned - next_base) + size;

        current->next->pos = new_pos;

        u8* arena_out = next_aligned;

        #ifdef DEBUG
        current->next->num_of_allocations++;
        #endif   /* DEBUG */

        return arena_out;
    }

    #ifdef DEBUG
    current->num_of_allocations++;
    #endif   /* DEBUG */

    current->pos = new_pos;

    u8* arena_out = (u8*)current->mem_region + align_pos;

    return arena_out;
}


#ifdef DEBUG    
#include <stdio.h>

void printArenaInfo(memory_arena* arena, b8 Printcurrent) {
    if (!arena || !arena->mem_region) {
        printf("NULL ARENA\n");
        return;
    }
    u64 alloc_num = 0;
    memory_arena* current = arena;
    if (Printcurrent) {
        while (current->next != NULL) {
            current = current->next;
            alloc_num++;
        }
        printf("Arena #%-2llu |allocs %-4llu|   |cap %-10llu|   |pos %-10llu / %-10llu|   |region reserved %p --> %p|\n",
        alloc_num, current->num_of_allocations, current->capacity, current->pos, current->capacity, current->mem_region, (u8*)current->mem_region + arena->capacity);
        return;
    }
    while (current != NULL) {
        printf("Arena #%-2llu |allocs %-4llu|   |cap %-10llu|   |pos %-10llu / %-10llu|   |region reserved %p --> %p|\n",
        ++alloc_num, current->num_of_allocations, current->capacity, current->pos, current->capacity, current->mem_region, (u8*)current->mem_region + arena->capacity);
        current = current->next;
    }
}

#endif  /* DEBUG */
