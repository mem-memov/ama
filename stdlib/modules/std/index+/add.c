/**
 * @file
 * @brief Add two retained index values after an exact overflow precheck.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_e6eb9bf5d06f30980699ebda75a1242796d5561c26666fd4d64d82b6b4491c7a.interface.h"
#include "ama_n_e6eb9bf5d06f30980699ebda75a1242796d5561c26666fd4d64d82b6b4491c7a.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_67b2a137d135b2b1fbfb5ba04486d5fb2fd03ded2402b2d01f83903a64805025 *parameters, ama_t_0c68a43b42ec61d8b0ec8824eb85444782138b68d94b4fe20dbdf3af5f6e546c *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t left = parameters->AMA_INPUT(0)->payload->value;
    size_t right = parameters->AMA_INPUT(1)->payload->value;
    if (left > SIZE_MAX - right) { *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); return; }
    results->AMA_OUTPUT(0)->payload->value = left + right;
    *status = ama_status_success();
}

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
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

#include "ama_n_e6eb9bf5d06f30980699ebda75a1242796d5561c26666fd4d64d82b6b4491c7a.end.h"
