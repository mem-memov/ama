#include <stdint.h>
#include <string.h>
#include "internal.h"

void source_manager_init(SourceManager *manager, SourceAllocator allocator) {
    *manager = (SourceManager){allocator, NULL};
}

void source_text_release(SourceAllocator allocator, SourceText *source) {
    if (source == NULL) {
        return;
    }
    allocator.release(allocator.context, source->line_starts);
    allocator.release(allocator.context, source->bytes);
    allocator.release(allocator.context, source->path);
    allocator.release(allocator.context, source);
}

void source_manager_destroy(SourceManager *manager) {
    while (manager->first != NULL) {
        SourceText *next = manager->first->next;
        source_text_release(manager->allocator, manager->first);
        manager->first = next;
    }
}

static SourceStatus copy_text(SourceManager *manager, SourceText *source,
    const char *path, const unsigned char *bytes, size_t length) {
    size_t path_length = strlen(path);
    if (path_length == SIZE_MAX || length == SIZE_MAX) {
        return SOURCE_LIMIT;
    }
    SourceAllocator allocator = manager->allocator;
    source->path = allocator.allocate(allocator.context, path_length + 1);
    source->bytes = allocator.allocate(allocator.context, length + 1);
    if (source->path == NULL || source->bytes == NULL) {
        return SOURCE_OUT_OF_MEMORY;
    }
    memcpy(source->path, path, path_length + 1);
    if (length != 0) {
        memcpy(source->bytes, bytes, length);
    }
    source->bytes[length] = 0;
    source->length = length;
    return source_text_lines(allocator, source);
}

SourceStatus source_manager_add(SourceManager *manager, const char *path,
    const unsigned char *bytes, size_t length, const SourceText **output) {
    if (path == NULL || output == NULL || (bytes == NULL && length != 0)) {
        return SOURCE_INVALID_ARGUMENT;
    }
    SourceAllocator allocator = manager->allocator;
    SourceText *source = allocator.allocate(allocator.context, sizeof(*source));
    if (source == NULL) {
        return SOURCE_OUT_OF_MEMORY;
    }
    *source = (SourceText){0};
    SourceStatus status = copy_text(manager, source, path, bytes, length);
    if (status != SOURCE_OK) {
        source_text_release(allocator, source);
        return status;
    }
    source->next = manager->first;
    manager->first = source;
    *output = source;
    return SOURCE_OK;
}
