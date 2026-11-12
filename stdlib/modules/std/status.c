/**
 * @file
 * @brief Own a retained immutable native status view with success default and value cloning.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_d8ee4a39419166db9c9c7c32b719f46ef4cb0ca4eefa92e05791136eec4acb67.interface.h"
#include "ama_n_d8ee4a39419166db9c9c7c32b719f46ef4cb0ca4eefa92e05791136eec4acb67.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_27bacea3c519f66c74472afd298c1ae3baf1c771de122843d3e65a145284c62f *parameters, ama_t_3fa05fc7c79cd3c802bfdce9becee5f41f8aab25b6824ad49c0e986910d0d4e8 *results,
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

#include "ama_n_d8ee4a39419166db9c9c7c32b719f46ef4cb0ca4eefa92e05791136eec4acb67.end.h"
