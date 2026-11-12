/**
 * @file
 * @brief Increment one retained index value after checking the maximum.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_d0622af49a8e62864b715c1bf3f56938b0759e7f710ce369e65af871199afa23.interface.h"
#include "ama_n_d0622af49a8e62864b715c1bf3f56938b0759e7f710ce369e65af871199afa23.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_3b1604e3d9e7019d0d3ef287ea105e2345cc4ba53050e11d0e31bc9a039f296e *parameters, ama_t_7804984acf19cc0398f69bfb6ac45849d434048193081c212ae235a52c3257bf *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t value = parameters->AMA_INPUT(0)->payload->value;
    if (value == SIZE_MAX) { *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); return; }
    results->AMA_OUTPUT(0)->payload->value = value + 1;
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

#include "ama_n_d0622af49a8e62864b715c1bf3f56938b0759e7f710ce369e65af871199afa23.end.h"
