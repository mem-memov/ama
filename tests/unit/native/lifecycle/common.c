/**
 * @file
 * @brief Declare the finite lifecycle failure model and its explicit resource ledger.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_LIFECYCLE_COMMON_C
#define AMA_TEST_LIFECYCLE_COMMON_C

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "model/project/payload.h"
#include "runtime/lifecycle/errors.h"

/** Own bounded child/native fixture resources, their lifetime ledger and observed destruction/diagnostic sequences. */
typedef struct {
    AmaLifetime lifetime;
    Payload native;
    Payload children[3];
    size_t destroyed[4];
    size_t destruction_count;
    AmaStatus diagnostics[4];
    size_t diagnostic_count;
} Model;

/** Explicit finite constructor/failure/cleanup choices; at most three children keep this reference-model boundary exhaustive. */
typedef struct {
    size_t children;
    AmaConstructorKind constructor;
    size_t failure_ordinal;
    unsigned cleanup_failures;
} Scenario;

#include "support/cleanup.c"
#include "support/construction.c"

/**
 * Assert the lifecycle model has no remaining native/child cleanup credits and is finalized
 * exactly as its scenario requires.
 */
static void expect_finalized(const Model *model) {
    assert(model->lifetime.phase == AMA_LIFETIME_FINALIZED);
    assert(!model->lifetime.cleanup_active && !model->lifetime.native_live);
    assert(model->lifetime.live_children == 0 && model->native.resource == NULL);
    for (size_t index = 0; index < 3; index++) {
        assert(model->children[index].resource == NULL);
    }
}

#endif
