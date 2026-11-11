#ifndef AMA_RUNTIME_METADATA_HOST_H
#define AMA_RUNTIME_METADATA_HOST_H

#include "identity.h"

typedef enum { AMA_HOST_COMPLETED, AMA_HOST_YIELDED, AMA_HOST_FAILED } AmaHostOutcome;

typedef struct {
    AmaHostOutcome outcome;
    AmaHandle root;
    AmaStatus status;
} AmaHostResult;

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
