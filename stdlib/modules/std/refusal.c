/**
 * @file
 * @brief Expose the immediate caller refusal snapshot as an ordinary returned status value.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_9cebd2822390cfe700b4b1749cf4cb389714be242fb7fcc3e63512378fd0e1f5.interface.h"
#include "ama_n_9cebd2822390cfe700b4b1749cf4cb389714be242fb7fcc3e63512378fd0e1f5.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_d1fb170f0c461b467f87e3db60ea6ce49ca92656bbdcab799c3b2188d1a22608 *parameters, ama_t_1251eb8f0a105c6bf76a3ffc38cf89afed7aaa869d52be5dfb17a9c638bf0680 *results,
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

#include "ama_n_9cebd2822390cfe700b4b1749cf4cb389714be242fb7fcc3e63512378fd0e1f5.end.h"
