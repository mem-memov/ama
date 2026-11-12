/**
 * @file
 * @brief Expose the immediate caller refusal snapshot as an ordinary returned status value.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_5e311a9cf1f92a83196f5dd28ba2ea70e210b11c7498bee2fb43b1805ee5eaa4.interface.h"
#include "ama_n_5e311a9cf1f92a83196f5dd28ba2ea70e210b11c7498bee2fb43b1805ee5eaa4.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_0ef1ce19a11994f99c0d593c908e6f4ebce966f14aa012436870961a785ac020 *parameters, ama_t_a37c0aaf4f8a49981a6f71341d4c74191a1ffc758e6bab0349c7dbb933f80c8c *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state;
    (void)parameters;
    results->AMA_OUTPUT(0)->payload->value = services->caller_refusal;
    *status = ama_status_success();
}

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    destination->reserved = 0;
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

#include "ama_n_5e311a9cf1f92a83196f5dd28ba2ea70e210b11c7498bee2fb43b1805ee5eaa4.end.h"
