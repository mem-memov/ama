/**
 * @file
 * @brief Create two distinct live local-instance metadata fixtures without payload allocation.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_METADATA_COMMON_C
#define AMA_TEST_METADATA_COMMON_C

#include <assert.h>
#include <stdlib.h>
#include "runtime/metadata/instance.h"
#include "runtime/metadata/host.h"
#include "runtime/metadata/source.h"
#include "runtime/metadata/runtime.h"

static AmaInstanceMetadata caller;
static AmaInstanceMetadata callee;

/**
 * Create fresh live metadata for one distinct local slot with complete zero-child lifecycle and
 * Ready service state; no payload pool allocation is implied.
 */
static AmaInstanceMetadata make_instance(size_t slot) {
    AmaInstanceMetadata instance = {0};
    instance.identity = (AmaSlotIdentity) {7, slot, 1, true, false};
    AmaStatus status = ama_lifetime_begin(&instance.lifetime, 0, AMA_CONSTRUCT_DEFAULT);
    assert(ama_status_is_success(status));
    status = ama_lifetime_finish(&instance.lifetime, false);
    assert(ama_status_is_success(status));
    instance.service.activity = AMA_INSTANCE_READY;
    return instance;
}

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    caller = make_instance(0);
    callee = make_instance(1);
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    caller = (AmaInstanceMetadata) {0};
    callee = (AmaInstanceMetadata) {0};
}

#endif
