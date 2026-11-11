#ifndef AMA_RUNTIME_LIFECYCLE_ERRORS_H
#define AMA_RUNTIME_LIFECYCLE_ERRORS_H

#include "native/status/status.h"

/* The caller reports each error separately; accumulation never allocates. */
static inline AmaStatus ama_cleanup_primary(AmaStatus primary, AmaStatus additional) {
    return ama_status_is_success(primary) ? additional : primary;
}

#endif
