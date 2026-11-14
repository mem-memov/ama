/**
 * @file
 * @brief Expose the immediate caller refusal snapshot as an ordinary returned status value.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the failure/resource library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 * [ERROR-06](../../../spec/implementation/diagnostics.md#error-06)
 * preserves the last-refusal snapshot as read-only ordinary value data.
 */
#include "ama_n_689872bd03618cf7e34cd8df67e542c0fc66610fcd957dfed79ea164fc9bfd11.interface.h"
#include "ama_n_689872bd03618cf7e34cd8df67e542c0fc66610fcd957dfed79ea164fc9bfd11.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_5e9d9a39482165aa76a1d7d56ae0814b0299c1c62edff9c258d064d677324f7a *parameters, ama_t_eb7262d6dab75b221998b7ba2f28a470504fcfd9ad504caa877dc8293f71113d *results,
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

#include "ama_n_689872bd03618cf7e34cd8df67e542c0fc66610fcd957dfed79ea164fc9bfd11.end.h"
