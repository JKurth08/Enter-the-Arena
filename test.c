#include "minunit.h"
#include "Arena.h"

/*
gcc test.c Arena.c -o tests -lm && ./tests
*/

/* alignment test */
MU_TEST(test_pointer_alignment) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* ptr = arena_alloc(arena, sizeof(int));
    *ptr = 50;
    mu_assert_int_eq(0, (uintptr_t)ptr % ARENA_ALIGN);

    arenaDestroy(arena);
}

MU_TEST(test_pointer_stability) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* a = arena_alloc(arena, sizeof(int));
    int* b = arena_alloc(arena, sizeof(int));
    int* ptr = b;
    arenaClear(arena);
    int* c = arena_alloc(arena, sizeof(int));
    int* d = arena_alloc(arena, sizeof(int));
    mu_assert(ptr == d, "pointer mis-match");

    arenaDestroy(arena);
}

/* mem tests */
MU_TEST(test_no_overlapping_mem_region) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* a = arena_alloc(arena, sizeof(int));
    int* b = arena_alloc(arena, sizeof(int));
    mu_assert((u8*)a + (ALIGN_UP(sizeof(int), ARENA_ALIGN)) <= (u8*)b, "Overlapping memory present, ABORT");

    arenaDestroy(arena);
}

MU_TEST(test_in_bounds_mem_region) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* a = arena_alloc(arena, sizeof(int));
    if ((u8*)arena->mem_region < (u8*)a) {
        mu_assert((u8*)a <= (u8*)arena->mem_region + arena->capacity, "pointer out of range");
    }

    arenaDestroy(arena);
}

MU_TEST(test_mem_region_cap_alloc) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* a = arena_alloc(arena, KiB(1));
    mu_assert((u8*)arena->mem_region + arena->capacity == (u8*)a + KiB(1), "edge case failed");

    arenaDestroy(arena);
}

MU_TEST(test_mem_region_not_null) {
    memory_arena* arena = arenaCreate(KiB(1));
    mu_assert(arena->mem_region != NULL, "Should be non NULL");

    arenaDestroy(arena);
}

/* Test using the stack */
MU_TEST(test_stack_init) {
    memory_arena arena;
    char data[KiB(1)];
    arenaInit(&arena, KiB(1), data);
    mu_assert_int_eq(0, arena.pos);
    mu_assert_int_eq(KiB(1), arena.capacity);
    mu_assert((u8*)arena.mem_region == (u8*)data, "mem region should point to data");
}

/* basic tests for setup */
MU_TEST(test_arena_non_null) {
    memory_arena* arena = arenaCreate(KiB(1));
    mu_assert(arena != NULL, "arena should not be NULL");

    arenaDestroy(arena);
}

MU_TEST(test_alloc_non_null) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* x = arena_alloc(arena, sizeof(int));
    mu_assert(x != NULL, "allocation should not be NULL");

    arenaDestroy(arena);
}

MU_TEST(test_arena_create_zero) {
    memory_arena* arena = arenaCreate(0);
    mu_assert(arena == NULL, "arena should be null");
}

MU_TEST(test_alloc_null_arena) {
    int* x = arena_alloc(NULL, 64);
    mu_assert(x == NULL, "allocation should be null");
}

MU_TEST(test_alloc_null_size) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* x = arena_alloc(arena, 0);
    mu_assert(x == NULL, "allocation should be null");

    arenaDestroy(arena);
}

MU_TEST(test_arena_destory_null) {
    arenaDestroy(NULL);
    mu_assert(1, "arenaDestroy(NULL) should not crash");
}

MU_TEST(test_arena_clear_null) {
    arenaClear(NULL);
    mu_assert(1, "arenaClear(NULL) should not crash");
}

MU_TEST(test_alloc_greater_than_capacity) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* nums = arena_alloc(arena, KiB(1) * sizeof(int));
    mu_assert(nums == NULL, "nums should be null");

    arenaDestroy(arena);
}

MU_TEST(test_arena_destroy) {
    memory_arena* arena = arenaCreate(KiB(1));
    arena_alloc(arena, 32);
    arenaDestroy(arena);
    mu_assert(1, "arenaDestroy(arena) should not crash");
}

/* heap tests */
MU_TEST(test_alloc_equals_pos) {
    memory_arena* arena = arenaCreate(KiB(1));
    arena_alloc(arena, 39);
    mu_assert_int_eq(39, (int)arena->pos);

    arena_alloc(arena, 64);
    int expected = ALIGN_UP(39, ARENA_ALIGN) + 64;
    mu_assert_int_eq(expected, (int)arena->pos);

    arenaDestroy(arena);
}

MU_TEST(test_alloc_equals_pos_many) {
    memory_arena* arena = arenaCreate(sizeof(int) + 100 * (ALIGN_UP(sizeof(int), ARENA_ALIGN)));
    for (int i = 0; i < 100; i++) {
        int* ptr = arena_alloc(arena, sizeof(int));
    }
    int expected = sizeof(int) + 99 * (ALIGN_UP(sizeof(int), ARENA_ALIGN));
    mu_assert_int_eq(expected, (int)arena->pos);

    arenaDestroy(arena);
} 

MU_TEST(test_clear_resets_pos) {
    memory_arena* arena = arenaCreate(KiB(1));
    arena_alloc(arena, 64);
    arenaClear(arena);
    mu_assert_int_eq(0, (int)arena->pos);

    arenaDestroy(arena);
}

MU_TEST(test_alloc_read_back) {
    memory_arena *arena = arenaCreate(KiB(1));
    int* a = arena_alloc(arena, sizeof(int));
    *a = 32;
    int* b = arena_alloc(arena, sizeof(int));
    *b = 64;
    mu_assert_int_eq(32, *a);

    arenaDestroy(arena);
}

/* Chain testing: arena not null, pos and capacity */
MU_TEST(test_chained_arenas) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* num = arena_alloc(arena, 800);
    mu_assert(arena->next == NULL, "arena->next should be null");
    int* num1 = arena_alloc(arena, 512);
    mu_assert(arena->next != NULL, "arena->next is not initialized");
    int* num2 = arena_alloc(arena, 800);
    mu_assert(arena->next->next != NULL, "arena->next->next is not initialized");

    mu_assert_int_eq(arena->capacity, arena->next->capacity);
    mu_assert_int_eq(512, arena->next->pos);

    mu_assert_int_eq(arena->capacity, arena->next->next->capacity);
    mu_assert_int_eq(800, arena->next->next->pos);

    arenaDestroy(arena);
}

MU_TEST(test_chained_arenas_after_clear) {
    memory_arena* arena = arenaCreate(KiB(1));
    int* num = arena_alloc(arena, 800);
    int* num1 = arena_alloc(arena, 800);

    arenaClear(arena);

    mu_assert_int_eq(0, arena->next->pos);
    mu_assert_int_eq(0, arena->pos);

    int* num2 = arena_alloc(arena, 256);
    mu_assert_int_eq(256, arena->pos);

    arenaDestroy(arena);
}

MU_TEST_SUITE(arena_suite) {
    /* alignment test */
    MU_RUN_TEST(test_pointer_alignment);
    MU_RUN_TEST(test_pointer_stability);
    /* mem test */
    MU_RUN_TEST(test_no_overlapping_mem_region);
    MU_RUN_TEST(test_in_bounds_mem_region);
    MU_RUN_TEST(test_mem_region_cap_alloc);
    MU_RUN_TEST(test_mem_region_not_null);
    /* stack test */
    MU_RUN_TEST(test_stack_init);
    /* basic setup */
    MU_RUN_TEST(test_arena_non_null);
    MU_RUN_TEST(test_alloc_non_null);
    MU_RUN_TEST(test_arena_create_zero);
    MU_RUN_TEST(test_alloc_null_arena);
    MU_RUN_TEST(test_alloc_null_size);
    MU_RUN_TEST(test_arena_destory_null);
    MU_RUN_TEST(test_arena_clear_null);
    MU_RUN_TEST(test_alloc_greater_than_capacity);
    MU_RUN_TEST(test_arena_destroy);
    /* heap tests */
    MU_RUN_TEST(test_alloc_equals_pos);
    MU_RUN_TEST(test_alloc_equals_pos_many);
    MU_RUN_TEST(test_clear_resets_pos);
    MU_RUN_TEST(test_alloc_read_back);
    /* multiple arena tests */
    MU_RUN_TEST(test_chained_arenas);
    MU_RUN_TEST(test_chained_arenas_after_clear);
}

int main(void) {
    MU_RUN_SUITE(arena_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
