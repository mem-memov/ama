/**
 * @file
 * @brief Allocate aligned arena records and checked concatenated strings.
 *
 * [ARCH-05](../../../spec/implementation/compiler.md#arch-05)
 * Check representable sizes before allocation; failure never links a partial record.
 */
#include <stdint.h>
#include <string.h>
#include "arena.h"

/** Private allocation header; max_align_t makes the following user region suitably aligned. */
struct SourceAllocation {
    SourceAllocation *next;
    max_align_t alignment;
};

void source_arena_init(SourceArena *arena, SourceAllocator allocator) {
    *arena = (SourceArena){allocator, NULL};
}

void source_arena_destroy(SourceArena *arena) {
    while (arena->first != NULL) {
        SourceAllocation *next = arena->first->next;
        arena->allocator.release(arena->allocator.context, arena->first);
        arena->first = next;
    }
}

/** A header precedes each aligned allocation; link only after allocation succeeds, so teardown owns exactly the successful requests.
 */
void *source_arena_allocate(SourceArena *arena, size_t size) {
    if (size > SIZE_MAX - sizeof(SourceAllocation)) {
        return NULL;
    }
    SourceAllocation *allocation = arena->allocator.allocate(arena->allocator.context,
        sizeof(*allocation) + size);
    if (allocation == NULL) {
        return NULL;
    }
    allocation->next = arena->first;
    arena->first = allocation;
    return allocation + 1;
}

/** Measure before allocation and check every sum. Copy only into fresh arena storage, permitting input strings already owned by this arena.
 */
char *source_arena_join(SourceArena *arena, const char *left,
    const char *separator, const char *right) {
    size_t left_length = strlen(left);
    size_t separator_length = strlen(separator);
    size_t right_length = strlen(right);
    if (left_length == SIZE_MAX || separator_length > SIZE_MAX - left_length - 1
        || right_length > SIZE_MAX - left_length - separator_length - 1) {
        return NULL;
    }
    size_t total = left_length + separator_length + right_length;
    char *result = source_arena_allocate(arena, total + 1);
    if (result != NULL) {
        memcpy(result, left, left_length);
        memcpy(result + left_length, separator, separator_length);
        memcpy(result + left_length + separator_length, right, right_length + 1);
    }
    return result;
}

char *source_arena_copy(SourceArena *arena, const char *text) {
    return source_arena_join(arena, text, "", "");
}
