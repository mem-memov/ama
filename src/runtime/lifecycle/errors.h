/**
 * @file
 * @brief Preserve the primary failure while cleanup reports secondary failures.
 *
 * [LIFE-05](../../../spec/implementation/native-interface.md#life-05)
 * The first non-success status wins without allocation or message interpretation.
 */
#ifndef AMA_RUNTIME_LIFECYCLE_ERRORS_H
#define AMA_RUNTIME_LIFECYCLE_ERRORS_H

#include "native/status/status.h"

/* The caller reports each error separately; accumulation never allocates. */
/** Return the first non-success status; secondary cleanup cannot replace an earlier construction or execution failure. */
static inline AmaStatus ama_cleanup_primary(AmaStatus primary, AmaStatus additional) {
    return ama_status_is_success(primary) ? additional : primary;
}

#endif
