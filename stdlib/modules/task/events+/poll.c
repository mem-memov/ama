/** @file
 * @brief Observe retained completions without consuming a budget through a native resource lease.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * permits blocking only after the scheduler has checked its declared work and waits.
 * This method never executes Ama tasks or captures borrowed views in callbacks.
 */
#include "ama_n_eb4006b022d0e0f1b8ca0e54f088b94b1c83ec08cd7edbe1432487caee203e4d.interface.h"
#include "ama_n_eb4006b022d0e0f1b8ca0e54f088b94b1c83ec08cd7edbe1432487caee203e4d.begin.h"
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
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_75153a8da08ad682fe63786c78d7a92475dcc036df8f3e9f8c06aa765397a48d *parameters,
    ama_t_3d58556923654798a0a10254c69d9c90e41e6feebc3ad564dcba6883a12aa377 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    AmaNativeEventReadiness readiness = {0};
    *status = ama_native_event_service_poll(state->lease.resource, &readiness);
    if (status->code == AMA_OK) {
        results->AMA_OUTPUT(0)->payload->value = readiness.ready ? 1 : 0;
        results->AMA_OUTPUT(1)->payload->value = readiness.pending ? 1 : 0;
    }
}
#include "ama_n_eb4006b022d0e0f1b8ca0e54f088b94b1c83ec08cd7edbe1432487caee203e4d.end.h"
