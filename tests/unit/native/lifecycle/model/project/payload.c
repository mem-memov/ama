/**
 * @file
 * @brief Allocate and release native fixture resources with selected constructor and destructor failures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "payload.h"
#include <stdlib.h>

/**
 * Allocate one native fixture byte, record constructor kind and publish the owned pointer only
 * after allocation succeeds.
 */
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

/**
 * Construct one default native fixture resource; allocation failure creates no destructor
 * credit.
 */
AmaStatus payload_initialize(Payload *destination) {
    return allocate_payload(destination, 0, AMA_CONSTRUCT_DEFAULT);
}

/**
 * Require exactly one literal byte and allocate one owned native fixture resource with the
 * literal constructor kind.
 */
AmaStatus payload_literal(Payload *destination, AmaByteView literal) {
    if (literal.length != 1) {
        return ama_status_make(AMA_INVALID_LITERAL, NULL, 0);
    }
    return allocate_payload(destination, literal.data[0], AMA_CONSTRUCT_LITERAL);
}

/**
 * Require a valid live source resource and allocate an independent one-byte copy with clone
 * constructor identity.
 */
AmaStatus payload_clone(Payload *destination, const Payload *source) {
    return allocate_payload(destination, *source->resource, AMA_CONSTRUCT_CLONE);
}

/**
 * Deliberately construct then release its own partial resource and return allocation failure,
 * proving failed constructors must not be destroyed again.
 */
AmaStatus payload_initialize_failing(Payload *destination) {
    AmaStatus status = payload_initialize(destination);
    if (ama_status_is_success(status)) {
        free(destination->resource);
        destination->resource = NULL;
    }
    return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
}

/**
 * Release the fixture resource once, clear its pointer and return its selected injected
 * destructor outcome after cleanup.
 */
AmaStatus payload_destroy(Payload *payload) {
    uint32_t code = payload->fail_destroy ? AMA_NATIVE_FAILURE : AMA_OK;
    free(payload->resource);
    payload->resource = NULL;
    return ama_status_make(code, NULL, 0);
}
