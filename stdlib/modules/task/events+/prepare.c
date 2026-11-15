/** @file
 * @brief Execute the explicit prepare operation on a retained native service.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * allows native records only; Ama state is accessed solely through these generated borrows.
 */
#include "ama_n_e35618119a24d111dacf009d538abb8356fbcd96fab72d62817aa72d336975e3.interface.h"
#include "ama_n_e35618119a24d111dacf009d538abb8356fbcd96fab72d62817aa72d336975e3.begin.h"
#include "native/events/assembly.h"
/** Construct an initially unbound method; the owning constructor binds a native lease. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Retire method resource rights without executing Ama or releasing another method's lease. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    AmaNativeEventKeepalive lease = state->lease;
    state->lease = (AmaNativeEventKeepalive){0};
    if (lease.release != NULL) { lease.release(lease.resource); }
    return ama_status_success();
}
/** Execute one bounded native operation; every temporary owner is released before return. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_a56293a2e3f1dc432a35f503ceef8d33bd47378f9c326fe1c05d447f880b3f34 *parameters,
    ama_t_02e8d6de769c41ecd5ab5223c6d37587a38a823950383f4546d7b92ca324cd11 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    AmaNativeEventSubmission submission = {0};
    *status = ama_native_event_service_prepare(state->lease.resource, &submission);
    if (status->code == AMA_OK) { results->AMA_OUTPUT(0)->payload->token = submission.token; }
    ama_native_event_submission_drop(&submission);
}
#include "ama_n_e35618119a24d111dacf009d538abb8356fbcd96fab72d62817aa72d336975e3.end.h"
