/**
 * @file
 * @brief Bind an incompatible native index signature and require C type rejection.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "types.h"

/**
 * Deliberately expose the incompatible unsigned index result signature; the enclosing typed
 * pointer binding must be rejected by the C compiler.
 */
AmaStatus wrong(const Payload *state, unsigned *result) {
    *result = (unsigned) state->total;
    return ama_status_success();
}

AmaStatus (*binding)(const Payload *, size_t *) = wrong;
