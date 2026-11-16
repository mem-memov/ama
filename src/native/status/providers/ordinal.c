/**
 * @file
 * @brief Encode the complete nonzero 16-bit provider namespace domain without wrap.
 *
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) forbids
 * collisions. For index below UINT16_MAX, index+1 lies in 1..UINT16_MAX;
 * rejecting the remaining domain before narrowing proves no zero or reused ID.
 */
#include "internal.h"

/** Publish one bounded ordinal only after its whole-width capacity check. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04). */
AmaStatus native_status_provider_ordinal(size_t index, uint16_t *namespace_id) {
    if (index >= UINT16_MAX) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    if (namespace_id == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *namespace_id = (uint16_t)(index + 1);
    return ama_status_success();
}
