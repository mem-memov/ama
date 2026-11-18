/**
 * @file
 * @brief Preserve first static-inventory failures with original source sites and no reporting allocation.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires source-
 * backed diagnostics; failure must not require another allocation to explain it.
 */
#include "internal.h"

/** Allocation refusal never publishes a usable inventory or overwrites an
 * earlier failure. Contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04).
 */
bool static_memory_failure(StaticInventory *inventory) {
    if (inventory->status == STATIC_OK) {
        inventory->status = STATIC_OUT_OF_MEMORY;
        inventory->valid = false;
        inventory->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Static inventory could not allocate storage"};
    }
    return false;
}

/** Keep a constant message and borrowed original declaration sites. Subsequent
 * failures leave this first diagnostic intact; no fallback category is chosen.
 * Contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04).
 */
bool static_invalid(StaticInventory *inventory, SourceSpan site, const char *message,
    const StaticSymbol *previous) {
    if (inventory->status != STATIC_OK) { return false; }
    inventory->status = STATIC_INVALID;
    inventory->valid = false;
    inventory->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
        .message = message, .primary = {.span = site}};
    if (previous != NULL) {
        inventory->related = (DiagnosticRelated){.location = {.span = previous->site},
            .role = "previous static declaration", .name = previous->owner->definition->source->logical_path};
        inventory->error.related = &inventory->related;
        inventory->error.related_count = 1;
    }
    return false;
}
