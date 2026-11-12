/**
 * @file
 * @brief Declare structural outcomes for the retained generated host boundary.
 *
 * [RT-14](../../../spec/implementation/runtime.md#rt-14)
 * Validation checks encoding only, not context ownership or pool liveness. Local handles are not checked cross-component tokens.
 */
#ifndef AMA_RUNTIME_METADATA_HOST_H
#define AMA_RUNTIME_METADATA_HOST_H

#include "identity.h"

/** Explicit terminal/yield/complete outcome for the retained generated host API, not a foreign-component token contract. */
typedef enum { AMA_HOST_COMPLETED, AMA_HOST_YIELDED, AMA_HOST_FAILED } AmaHostOutcome;

/** Outcome plus local root and status; structural validity alone does not prove context ownership or root liveness. */
typedef struct {
    AmaHostOutcome outcome;
    AmaHandle root;
    AmaStatus status;
} AmaHostResult;

/** Check outcome, message view and local-root encoding only. This predicate cannot validate pool liveness, context identity or foreign-component tokens. */
static inline bool ama_host_result_is_valid(AmaHostResult result) {
    bool root_valid = ama_handle_is_empty(result.root) ||
        (result.root.type_id != 0 && result.root.generation != 0);
    if (!root_valid || !ama_status_has_valid_message_view(result.status)) {
        return false;
    }
    if (result.outcome == AMA_HOST_FAILED) {
        return !ama_status_is_success(result.status);
    }
    if (result.outcome != AMA_HOST_COMPLETED && result.outcome != AMA_HOST_YIELDED) {
        return false;
    }
    return ama_status_is_success(result.status) && result.root.type_id != 0 &&
        result.root.generation != 0;
}

#endif
