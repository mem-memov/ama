#ifndef AMA_SOURCE_ARENA_H
#define AMA_SOURCE_ARENA_H

#include "allocator.h"

typedef struct SourceAllocation SourceAllocation;
typedef struct {
    SourceAllocator allocator;
    SourceAllocation *first;
} SourceArena;

void source_arena_init(SourceArena *arena, SourceAllocator allocator);
void source_arena_destroy(SourceArena *arena);
void *source_arena_allocate(SourceArena *arena, size_t size);
char *source_arena_copy(SourceArena *arena, const char *text);
char *source_arena_join(SourceArena *arena, const char *left,
    const char *separator, const char *right);

#endif
