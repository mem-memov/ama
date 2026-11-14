/**
 * @file
 * @brief Allocate and release native fixture resources with selected constructor and destructor failures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "payload.h"
#include <stdlib.h>

static SourceAllocator payload_allocator;

/** Borrow the fixture allocator for one independent sequential scenario. */
void payload_configure_allocator(SourceAllocator allocator) { payload_allocator = allocator; }

/** Release a failed hook's own partial resource; the runtime must not destroy it again. */
void payload_release_failed(Payload *payload) {
    payload_allocator.release(payload_allocator.context, payload->resource);
    payload->resource = NULL;
}

/**
 * Allocate one native fixture byte, record constructor kind and publish the owned pointer only
 * after allocation succeeds.
 */
static AmaStatus allocate_payload(Payload *destination, unsigned char value,
    AmaConstructorKind kind) {
    *destination = (Payload) {0};
    destination->resource = payload_allocator.allocate(payload_allocator.context, 1);
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

/** Construct the fixture's bounded size value through its own selected constructor. */
AmaStatus payload_from_size(Payload *destination, size_t value) {
    if (value > 255) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    return allocate_payload(destination, (unsigned char)value, AMA_CONSTRUCT_SIZE);
}

/**
 * Deliberately construct then release its own partial resource and return allocation failure,
 * proving failed constructors must not be destroyed again.
 */
AmaStatus payload_initialize_failing(Payload *destination) {
    AmaStatus status = payload_initialize(destination);
    if (ama_status_is_success(status)) {
        payload_release_failed(destination);
    }
    return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
}

/**
 * Release the fixture resource once, clear its pointer and return its selected injected
 * destructor outcome after cleanup.
 */
AmaStatus payload_destroy(Payload *payload) {
    uint32_t code = payload->destroy_code != AMA_OK ? payload->destroy_code
        : payload->fail_destroy ? AMA_NATIVE_FAILURE : AMA_OK;
    payload_release_failed(payload);
    return ama_status_make(code, NULL, 0);
}

/** Construct one native resource through the explicitly selected transport fixture. */
AmaStatus payload_from_transport(Payload *destination) {
    return allocate_payload(destination, 7, AMA_CONSTRUCT_TRANSPORT);
}
