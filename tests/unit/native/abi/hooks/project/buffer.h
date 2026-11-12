/**
 * @file
 * @brief Declare native buffer views and an entry that reads lengths and immediate refusal snapshots.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef FIXTURE_BUFFER_H
#define FIXTURE_BUFFER_H

#include <stdbool.h>
#include "native/abi/bytes.h"
#include "native/abi/services.h"
#include "native/toolchain/gcc.h"

/** Own heap bytes and count entry uses; conversion/clone/destroy error fields select exact injected failure outcomes. */
typedef struct {
    unsigned char *data;
    size_t length;
    size_t entry_count;
    uint32_t conversion_error;
    uint32_t destroy_error;
    uint32_t clone_error;
} AMA_NATIVE(FixtureBuffer);

/** Scalar output storage for explicit buffer-length observations. */
typedef struct {
    size_t value;
} AMA_NATIVE(FixtureCount);

/** Borrow the immutable input buffer only for the synchronous entry. */
typedef struct {
    const AMA_NATIVE(FixtureBuffer) *value;
} AMA_NATIVE(FixtureInputs);

/** Borrow writable count/status result locations only for the synchronous entry. */
typedef struct {
    AMA_NATIVE(FixtureCount) *length;
    AmaStatus *refusal;
} AMA_NATIVE(FixtureOutputs);

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(FixtureBuffer) *destination);
/**
 * Release only successfully constructed native State. Retained scalar modules own no resource;
 * fixture-specific injected failures are returned after cleanup.
 */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(FixtureBuffer) *state);
/**
 * Allocate independent owned bytes from the exact bounded literal; initialize failed
 * destinations safely and retain no borrowed literal pointer.
 */
AmaStatus AMA_NATIVE(from_literal)(AMA_NATIVE(FixtureBuffer) *destination, AmaByteView literal);
/**
 * Copy source native data into fresh destination storage without copying runtime identity or
 * transient execution links; source stays unchanged.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(FixtureBuffer) *destination, const AMA_NATIVE(FixtureBuffer) *source);
/**
 * Read buffer length through the explicit condition hook; return the selected injected technical
 * failure without mutating native resources.
 */
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(FixtureBuffer) *state, bool *result);
/**
 * Read buffer length through the explicit index hook; preserve the distinct injected
 * invalid-index/technical status for caller classification.
 */
AmaStatus AMA_NATIVE(to_index)(const AMA_NATIVE(FixtureBuffer) *state, size_t *result);

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(FixtureBuffer) *state, const AMA_NATIVE(FixtureInputs) *parameters,
    AMA_NATIVE(FixtureOutputs) *results, const AmaNativeServices *services, AmaStatus *status) {
    if (state->entry_count == SIZE_MAX) {
        *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
        return;
    }
    state->entry_count++;
    results->length->value = parameters->value->length;
    *results->refusal = services->caller_refusal;
    *status = ama_status_success();
}

#endif
