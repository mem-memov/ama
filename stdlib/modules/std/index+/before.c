/**
 * @file
 * @brief Compare two retained index values and produce a binary condition result.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_fa5fc8080167d3522b7026ef0e769931784daaba468d86cde051a23e6be0218b.interface.h"
#include "ama_n_fa5fc8080167d3522b7026ef0e769931784daaba468d86cde051a23e6be0218b.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_70846f4335593d2dca55b07f4f5996fa26970eaee131109f6074d1f6d4ff136d *parameters, ama_t_2c170170deb7e3e68ba3b241c09ca4950fa8db4cdcb2414c4dc1c6c190b5704f *results,
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

#include "ama_n_fa5fc8080167d3522b7026ef0e769931784daaba468d86cde051a23e6be0218b.end.h"
