#include "types.h"

AmaStatus wrong(const Payload *state, unsigned *result) {
    *result = (unsigned) state->total;
    return ama_status_success();
}

AmaStatus (*binding)(const Payload *, size_t *) = wrong;
