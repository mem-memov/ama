/**
 * @file
 * @brief Compose caller-supplied nonzero provider/reason pairs into disjoint library codes.
 *
 * [ERROR-03](../../../spec/implementation/diagnostics.md#error-03)
 * The helper checks the pair only; compiler dependency registration must establish
 * provider uniqueness before emission. Checked transport does not register providers.
 */
#ifndef AMA_NATIVE_STATUS_LIBRARY_H
#define AMA_NATIVE_STATUS_LIBRARY_H

#include "status.h"

/* Callers supply an already unique provider namespace; automatic registration
 * belongs to compiler dependency planning. Reject zero fields without changing output. */
/** Require caller-assigned unique provider identity and nonzero reason, both 16-bit.
 * @param[out] output Changes only on success; caller supplies writable storage.
 * @return Invalid state for either zero field, otherwise the collision-free concatenation (namespace << 16) | reason. No namespace registry is implemented here.
 * @param[in] namespace_id Caller-assigned unique nonzero 16-bit provider namespace; this helper does not register providers.
 * @param[in] reason Nonzero 16-bit provider-local reason, concatenated without arithmetic overlap.
 */
static inline AmaStatus ama_library_status_code(uint16_t namespace_id,
    uint16_t reason, uint32_t *output) {
    if (namespace_id == 0 || reason == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *output = ((uint32_t) namespace_id << 16) | (uint32_t) reason;
    return ama_status_success();
}

#endif
