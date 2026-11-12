/**
 * @file
 * @brief Own aligned scratch allocations until deterministic arena teardown.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Keep operation-local memory lifetime explicit and independently testable.
 */
#ifndef AMA_SOURCE_ARENA_H
#define AMA_SOURCE_ARENA_H

#include "allocator.h"

/**
 * Opaque arena-owned allocation chain node; source clients can allocate without depending on its
 * private representation.
 */
typedef struct SourceAllocation SourceAllocation;
/** Owner of independently allocated aligned records; returned addresses remain stable until destroy. */
typedef struct {
    SourceAllocator allocator;
    SourceAllocation *first;
} SourceArena;

/** Initialize an empty owner; allocator callbacks/context must outlive the arena.
 */
void source_arena_init(SourceArena *arena, SourceAllocator allocator);
/** Release every allocation once and leave the arena reusable. All returned pointers expire.
 */
void source_arena_destroy(SourceArena *arena);
/** Reserve max_align_t-aligned storage, including a valid zero-size request.
 * @return Arena-owned pointer, or null on checked-size/allocation failure without changing the allocation chain.
 */
void *source_arena_allocate(SourceArena *arena, size_t size);
/** Copy a borrowed NUL-terminated string; return arena-owned storage or null.
 */
char *source_arena_copy(SourceArena *arena, const char *text);
/** Concatenate three borrowed NUL-terminated strings with checked total/terminator size.
 * @return Arena-owned string or null; inputs and existing allocations remain unchanged on failure.
 */
char *source_arena_join(SourceArena *arena, const char *left,
    const char *separator, const char *right);

#endif
