/**
 * @file
 * @brief Verify explicit arena allocation authority, alignment and failed-prefix cleanup.
 */
#include <stdint.h>
#include "../common.c"
#include "source/memory/arena.h"

/** Nonzero/zero allocations stay max-aligned, stable and owned through bulk teardown. */
static void it_keeps_aligned_addresses_stable_and_releases_every_record(void) {
    setup_test();
    SourceArena arena;
    source_arena_init(&arena, sources.allocator);
    unsigned char *first = source_arena_allocate(&arena, 7);
    assert(first != NULL && (uintptr_t)first % _Alignof(max_align_t) == 0);
    memset(first, 0x57, 7);
    for (size_t index = 0; index < 20; index++) {
        void *value = source_arena_allocate(&arena, index);
        assert(value != NULL && (uintptr_t)value % _Alignof(max_align_t) == 0);
    }
    for (size_t index = 0; index < 7; index++) { assert(first[index] == 0x57); }
    source_arena_destroy(&arena);
    assert(arena.first == NULL && live_allocations == 0);
    source_arena_destroy(&arena);
    destroy_test();
}

/** Checked limits and injected allocator failure do not disturb existing owned strings. */
static void it_preserves_existing_strings_on_limit_and_allocation_failure(void) {
    setup_test();
    SourceArena arena;
    source_arena_init(&arena, sources.allocator);
    char *first = source_arena_copy(&arena, "original");
    assert(first != NULL);
    size_t before = allocation_count;
    assert(source_arena_allocate(&arena, SIZE_MAX) == NULL && allocation_count == before);
    failure_at = before + 1;
    assert(source_arena_join(&arena, first, "/", "next") == NULL);
    assert(strcmp(first, "original") == 0 && live_allocations == 1);
    failure_at = 0;
    char *joined = source_arena_join(&arena, first, "/", first);
    assert(joined != NULL && strcmp(joined, "original/original") == 0);
    source_arena_destroy(&arena);
    assert(live_allocations == 0);
    destroy_test();
}

/** The production default authority needs no context and accepts null releases. */
static void it_uses_the_system_authority_without_global_compiler_state(void) {
    SourceAllocator allocator = source_system_allocator();
    allocator.release(allocator.context, NULL);
    SourceArena arena;
    source_arena_init(&arena, allocator);
    char *empty = source_arena_copy(&arena, "");
    char *value = source_arena_join(&arena, "one", "-", "two");
    assert(empty != NULL && *empty == 0 && value != NULL && strcmp(value, "one-two") == 0);
    source_arena_destroy(&arena);
}

void run(void) {
    it_keeps_aligned_addresses_stable_and_releases_every_record();
    it_preserves_existing_strings_on_limit_and_allocation_failure();
    it_uses_the_system_authority_without_global_compiler_state();
}

int main(void) {
    run();
    return 0;
}
