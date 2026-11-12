/**
 * @file
 * @brief Compare two retained index values and produce a binary condition result.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_1f65df889e9651be26f93dd77aaeb32b0b4dff69a779ce533964a874fbdb7dbd.interface.h"
#include "ama_n_1f65df889e9651be26f93dd77aaeb32b0b4dff69a779ce533964a874fbdb7dbd.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_b0b283f85496c21cc3cef8cef581a4073a3cb36c135fd2c53e8017f471537cc2 *parameters, ama_t_f78ce0059cb8ac8f642db0ba036f79d0e72fb1061b9390301bc33e68e2df21ac *results,
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

#include "ama_n_1f65df889e9651be26f93dd77aaeb32b0b4dff69a779ce533964a874fbdb7dbd.end.h"
