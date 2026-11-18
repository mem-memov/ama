/**
 * @file
 * @brief Retain first static-term construction errors without allocating diagnostic storage.
 *
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps source
 * locations original even when term/environment construction exhausts storage.
 */
#include "internal.h"

/** Allocation refusal cannot overwrite an earlier source/identity error.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_term_memory_failure(StaticTermStore *store) {
    if (store->status == STATIC_TERM_OK) {
        store->status = STATIC_TERM_OUT_OF_MEMORY;
        store->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Static term construction could not allocate storage"};
    }
    return false;
}

/** Failure only changes this owner; input inventories remain accepted/immutable.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_term_invalid(StaticTermStore *store, SourceSpan site, const char *message) {
    if (store->status == STATIC_TERM_OK) {
        store->status = STATIC_TERM_INVALID;
        store->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
            .message = message, .primary = {.span = site}};
    }
    return false;
}
