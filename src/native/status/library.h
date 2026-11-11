#ifndef AMA_NATIVE_STATUS_LIBRARY_H
#define AMA_NATIVE_STATUS_LIBRARY_H

#include "status.h"

/* The compiler assigns unique nonzero namespaces by sorted provider identity.
 * Reject zero namespace/reason without changing the supplied output.
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
