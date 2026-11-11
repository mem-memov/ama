#include <stdint.h>
#include <string.h>
#include "arena.h"

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
