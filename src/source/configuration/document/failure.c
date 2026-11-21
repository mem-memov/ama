/**
 * @file
 * @brief Distinguish schema refusal from arena allocation refusal with original sites.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains partial
 * records for teardown; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * keeps failures attributable to the source configuration entry.
 */
#include "internal.h"

bool configuration_document_failure(ConfigurationDocument *document, SourceSpan span,
    const char *message, const char *expected) {
    document->status = CONFIGURATION_INVALID;
    return configuration_record_failure(&document->issue, span, message, expected);
}

void *configuration_document_allocate(ConfigurationDocument *document, size_t size, SourceSpan span) {
    void *record = source_arena_allocate(&document->arena, size);
    if (record == NULL) {
        document->status = CONFIGURATION_OUT_OF_MEMORY;
        (void)configuration_record_failure(&document->issue, span,
            "Configuration allocation failed", "available record storage");
    }
    return record;
}
