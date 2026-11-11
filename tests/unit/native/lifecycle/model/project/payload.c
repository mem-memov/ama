#include "payload.h"
#include <stdlib.h>

static AmaStatus allocate_payload(Payload *destination, unsigned char value,
    AmaConstructorKind kind) {
    *destination = (Payload) {0};
    destination->resource = malloc(1);
    if (destination->resource == NULL) {
        return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
    }
    *destination->resource = value;
    destination->constructed_by = kind;
    return ama_status_success();
}

AmaStatus payload_initialize(Payload *destination) {
    return allocate_payload(destination, 0, AMA_CONSTRUCT_DEFAULT);
}

AmaStatus payload_literal(Payload *destination, AmaByteView literal) {
    if (literal.length != 1) {
        return ama_status_make(AMA_INVALID_LITERAL, NULL, 0);
    }
    return allocate_payload(destination, literal.data[0], AMA_CONSTRUCT_LITERAL);
}

AmaStatus payload_clone(Payload *destination, const Payload *source) {
    return allocate_payload(destination, *source->resource, AMA_CONSTRUCT_CLONE);
}

AmaStatus payload_initialize_failing(Payload *destination) {
    AmaStatus status = payload_initialize(destination);
    if (ama_status_is_success(status)) {
        free(destination->resource);
        destination->resource = NULL;
    }
    return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
}

AmaStatus payload_destroy(Payload *payload) {
    uint32_t code = payload->fail_destroy ? AMA_NATIVE_FAILURE : AMA_OK;
    free(payload->resource);
    payload->resource = NULL;
    return ama_status_make(code, NULL, 0);
}
