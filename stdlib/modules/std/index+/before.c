/**
 * @file
 * @brief Compare two retained index values and produce a binary condition result.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the scalar library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 * [LIB-01](../../../../spec/language/library.md#lib-01)
 * binds this native scalar by its explicit standard-root nominal identity.
 */
#include "ama_n_88ac7c436c4104b856a6eb014d9c462a640142cfda712c5ae13b2c2f0f5f0619.interface.h"
#include "ama_n_88ac7c436c4104b856a6eb014d9c462a640142cfda712c5ae13b2c2f0f5f0619.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_f31e4a30c3e837c8d50c54ed853ba164c9445b7f2f2ec7d96e3346175fe637f1 *parameters, ama_t_468716a115a0d9f2da797bfa1a49cc8e3b0a933362c80804e8a7f6f3b6e9bd79 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    results->AMA_OUTPUT(0)->payload->value = parameters->AMA_INPUT(0)->payload->value < parameters->AMA_INPUT(1)->payload->value;
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

#include "ama_n_88ac7c436c4104b856a6eb014d9c462a640142cfda712c5ae13b2c2f0f5f0619.end.h"
