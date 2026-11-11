#ifndef AMA_SOURCE_ALLOCATOR_H
#define AMA_SOURCE_ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

typedef struct {
    void *context;
    void *(*allocate)(void *context, size_t size);
    void (*release)(void *context, void *allocation);
} SourceAllocator;

static inline void *source_system_allocate(void *context, size_t size) {
    (void)context;
    return malloc(size);
}

static inline void source_system_release(void *context, void *allocation) {
    (void)context;
    free(allocation);
}

static inline SourceAllocator source_system_allocator(void) {
    return (SourceAllocator){NULL, source_system_allocate, source_system_release};
}

#endif
