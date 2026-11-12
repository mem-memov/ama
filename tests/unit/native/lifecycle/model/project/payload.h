/**
 * @file
 * @brief Declare the lifecycle fixture resource and its constructor/destructor test hooks.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef FIXTURE_PAYLOAD_H
#define FIXTURE_PAYLOAD_H

#include "native/abi/bytes.h"
#include "runtime/lifecycle/lifetime.h"

/** Own one allocated native byte, its selected constructor identity and an explicit post-cleanup failure switch. */
typedef struct {
    unsigned char *resource;
    AmaConstructorKind constructed_by;
    bool fail_destroy;
} Payload;

/**
 * Construct one default native fixture resource; allocation failure creates no destructor
 * credit.
 */
AmaStatus payload_initialize(Payload *destination);
/**
 * Require exactly one literal byte and allocate one owned native fixture resource with the
 * literal constructor kind.
 */
AmaStatus payload_literal(Payload *destination, AmaByteView literal);
/**
 * Require a valid live source resource and allocate an independent one-byte copy with clone
 * constructor identity.
 */
AmaStatus payload_clone(Payload *destination, const Payload *source);
/**
 * Deliberately construct then release its own partial resource and return allocation failure,
 * proving failed constructors must not be destroyed again.
 */
AmaStatus payload_initialize_failing(Payload *destination);
/**
 * Release the fixture resource once, clear its pointer and return its selected injected
 * destructor outcome after cleanup.
 */
AmaStatus payload_destroy(Payload *payload);

#endif
