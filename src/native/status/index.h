#ifndef AMA_NATIVE_STATUS_INDEX_H
#define AMA_NATIVE_STATUS_INDEX_H

#include "status.h"

typedef enum {
    AMA_INDEX_CONVERTED,
    AMA_INDEX_REFUSED,
    AMA_INDEX_FAILED
} AmaIndexOutcome;

/* Only the invalid-index code is a conversion refusal; other errors are fatal. */
static inline AmaIndexOutcome ama_index_outcome(AmaStatus status) {
    if (ama_status_is_success(status)) {
        return AMA_INDEX_CONVERTED;
    }
    return status.code == AMA_INVALID_INDEX ? AMA_INDEX_REFUSED : AMA_INDEX_FAILED;
}

#endif
