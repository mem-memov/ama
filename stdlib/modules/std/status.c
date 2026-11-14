/**
 * @file
 * @brief Own a retained immutable native status view with success default and value cloning.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the failure/resource library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 * [ERROR-06](../../../spec/implementation/diagnostics.md#error-06)
 * preserves the last-refusal snapshot as read-only ordinary value data.
 */
#include "ama_n_241273948e6d356c9be3b1bdbe2e9db07e56ce1d5f012e08816ba65437df2cfa.interface.h"
#include "ama_n_241273948e6d356c9be3b1bdbe2e9db07e56ce1d5f012e08816ba65437df2cfa.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_92d3a6b70bcd311e0e2f9fea7ea4b1bac33f59b848b5284497ccf6b27555a569 *parameters, ama_t_6836387c81c4095d25618e98cc8ab7c8a66210b8e127bfcfec5b4e8deb6f18e5 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state;
    (void)parameters;
    (void)results;
    (void)services;
    *status = ama_status_success();
}

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    destination->value = ama_status_success();
    return ama_status_success();
}

/**
 * Release only successfully constructed native State. Retained scalar modules own no resource;
 * fixture-specific injected failures are returned after cleanup.
 */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}

/**
 * Copy source native data into fresh destination storage without copying runtime identity or
 * transient execution links; source stays unchanged.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}

#include "ama_n_241273948e6d356c9be3b1bdbe2e9db07e56ce1d5f012e08816ba65437df2cfa.end.h"
