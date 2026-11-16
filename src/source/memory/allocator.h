/**
 * @file
 * @brief Pass explicit allocation/release authority across compiler components.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Allocator lifetime and failure are explicit; no mutable compiler singleton is needed.
 */
#ifndef AMA_SOURCE_ALLOCATOR_H
#define AMA_SOURCE_ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

/** Borrowed malloc/free-compatible authority. release accepts null; allocation returns suitably aligned storage or null. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
typedef struct {
    void *context;
    void *(*allocate)(void *context, size_t size);
    void (*release)(void *context, void *allocation);
} SourceAllocator;

/** Provide malloc-compatible allocation; context is unused and null reports allocation failure.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
static inline void *source_system_allocate(void *context, size_t size) {
    (void)context;
    return malloc(size);
}

/** Release a malloc-owned pointer or null; context is unused.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
static inline void source_system_release(void *context, void *allocation) {
    (void)context;
    free(allocation);
}

/** Return stateless malloc/free callbacks whose lifetime is the process lifetime.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
static inline SourceAllocator source_system_allocator(void) {
    return (SourceAllocator){NULL, source_system_allocate, source_system_release};
}

#endif
