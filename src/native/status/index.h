/**
 * @file
 * @brief Separate converted, explicitly invalid and technically failed native index outcomes.
 *
 * [ERROR-05](../../../spec/implementation/diagnostics.md#error-05)
 * Only AMA_INVALID_INDEX denotes conversion refusal; unrelated errors never become false or bounds success.
 */
#ifndef AMA_NATIVE_STATUS_INDEX_H
#define AMA_NATIVE_STATUS_INDEX_H

#include "status.h"

/** Conversion classification is distinct from target acquisition and never inferred from diagnostic wording. Component contract: [ERROR-05](../../../spec/implementation/diagnostics.md#error-05). */
typedef enum {
    AMA_INDEX_CONVERTED,
    AMA_INDEX_REFUSED,
    AMA_INDEX_FAILED
} AmaIndexOutcome;

/* Only the invalid-index code is a conversion refusal; other errors are fatal. */
/** Check transport shape, then classify the authoritative code. Success converts,
 * exactly AMA_INVALID_INDEX refuses, and every other failure remains technical;
 * malformed success/refusal cannot permit output consumption. Text is irrelevant.
 * Component contract: [ERROR-05](../../../spec/implementation/diagnostics.md#error-05). */
static inline AmaIndexOutcome ama_index_outcome(AmaStatus status) {
    status = ama_status_checked(status);
    if (ama_status_is_success(status)) {
        return AMA_INDEX_CONVERTED;
    }
    return status.code == AMA_INVALID_INDEX ? AMA_INDEX_REFUSED : AMA_INDEX_FAILED;
}

#endif
