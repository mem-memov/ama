/** @file
 * @brief Fail a real exported child instance constructor while its context is already live.
 */
#include "broken-api.h"
#include AMA_BROKEN_INTERFACE_HEADER
#include AMA_BROKEN_BEGIN_HEADER
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    state->unused=false;
    return ama_status_make(AMA_OUT_OF_MEMORY,"Deliberate child constructor failure",sizeof("Deliberate child constructor failure")-1);
}
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** This body is unreachable because no default instance can be constructed. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_BROKEN_PARAMETERS *parameters, AMA_BROKEN_RESULTS *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status=ama_status_make(AMA_INVALID_STATE,NULL,0);
}
#include AMA_BROKEN_END_HEADER
