/** @file
 * @brief Wait for retained completions through a native resource lease.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * permits blocking only after the scheduler has checked its declared work and waits.
 * This method never executes Ama tasks or captures borrowed views in callbacks.
 */
#include "ama_n_5e78ff5ae390d12708d364d340cb58b5c05787b361ebb62f36166745cde86f71.interface.h"
#include "ama_n_5e78ff5ae390d12708d364d340cb58b5c05787b361ebb62f36166745cde86f71.begin.h"
#include "native/events/assembly.h"
/** Create an unbound method; only its owner constructor supplies the native lease. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Release only this method's native credit after its activation has ended. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    AmaNativeEventKeepalive lease = state->lease;
    state->lease = (AmaNativeEventKeepalive){0};
    if (lease.release != NULL) { lease.release(lease.resource); }
    return ama_status_success();
}
/** Keep every generated borrow inside this activation; callbacks receive native resources only. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_21ddff0ec8208490a7d4055bf2cd27beb5ddf098b49d938b23931be9e19b4441 *parameters,
    ama_t_87ba4811dbd2287246e5a0690718834844a1f5bb6cdf5be3448a69cd11b9f280 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    bool ready = false;
    *status = ama_native_event_service_wait(state->lease.resource, &ready);
    if (status->code == AMA_OK) { results->AMA_OUTPUT(0)->payload->value = ready ? 1 : 0; }
}
#include "ama_n_5e78ff5ae390d12708d364d340cb58b5c05787b361ebb62f36166745cde86f71.end.h"
