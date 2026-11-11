#ifndef FIXTURE_PAYLOAD_H
#define FIXTURE_PAYLOAD_H

#include "native/abi/bytes.h"
#include "runtime/lifecycle/lifetime.h"

typedef struct {
    unsigned char *resource;
    AmaConstructorKind constructed_by;
    bool fail_destroy;
} Payload;

AmaStatus payload_initialize(Payload *destination);
AmaStatus payload_literal(Payload *destination, AmaByteView literal);
AmaStatus payload_clone(Payload *destination, const Payload *source);
AmaStatus payload_initialize_failing(Payload *destination);
AmaStatus payload_destroy(Payload *payload);

#endif
