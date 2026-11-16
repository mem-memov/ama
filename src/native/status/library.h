/**
 * @file
 * @brief Compose caller-supplied nonzero provider/reason pairs into disjoint library codes.
 *
 * [ERROR-03](../../../spec/implementation/diagnostics.md#error-03)
 * Helpers check numeric pairs and message shape; compiler dependency planning
 * establishes provider uniqueness before emission. Checked transport does not
 * register providers. [ERROR-04](../../../spec/implementation/diagnostics.md#error-04)
 * keeps message storage borrowed and program-lived even when constructing errors.
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
 * Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline AmaStatus ama_library_status_code(uint16_t namespace_id,
    uint16_t reason, uint32_t *output) {
    if (namespace_id == 0 || reason == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *output = ((uint32_t) namespace_id << 16) | (uint32_t) reason;
    return ama_status_success();
}

/** Build one module-qualified native error without allocating or retaining message storage.
 * Namespace assignment is compiler-validated; reason is the provider's nonzero
 * 16-bit local code. Invalid pair/message shape becomes ordinary technical InvalidState.
 * Message bytes retain the immutable program-lifetime native ABI precondition.
 * Component contract: [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), [ERROR-04](../../../spec/implementation/diagnostics.md#error-04). */
static inline AmaStatus ama_library_status(uint16_t namespace_id, uint16_t reason,
    const char *message, size_t length) {
    uint32_t code = 0;
    AmaStatus status = ama_library_status_code(namespace_id, reason, &code);
    return status.code == AMA_OK ? ama_status_checked(ama_status_make(code, message, length)) : status;
}

#endif
