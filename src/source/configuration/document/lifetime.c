/**
 * @file
 * @brief Retain one arena cleanup path for complete and refused INI documents.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires exact
 * ownership: source views are borrowed, while all record allocations retire here.
 */
#include "document.h"

void configuration_document_init(ConfigurationDocument *document, SourceAllocator allocator) {
    *document = (ConfigurationDocument){0};
    source_arena_init(&document->arena, allocator);
}

void configuration_document_destroy(ConfigurationDocument *document) {
    source_arena_destroy(&document->arena);
    *document = (ConfigurationDocument){0};
}
