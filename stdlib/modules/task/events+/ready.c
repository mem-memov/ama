/** @file
 * @brief Execute the explicit ready operation on a retained native service.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * allows native records only; Ama state is accessed solely through these generated borrows.
 */
#include "ama_n_01e818c7085dcd0bf0eae8737eaf6e1d2c8228cbbf230029ff59fa8e039b1e6a.interface.h"
#include "ama_n_01e818c7085dcd0bf0eae8737eaf6e1d2c8228cbbf230029ff59fa8e039b1e6a.begin.h"
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
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_dffdbc228202d52dfa61226a3be9e8a222aad7409741bd305aeb2bc4ed57b367 *parameters,
    ama_t_ae4860da7d33c9b2355f0b1049c6e80d50d9e8eb77e664a53f69c62d08c75386 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    AmaNativeEventSubmission submission = {0};
    *status = ama_native_event_service_callback(state->lease.resource, parameters->AMA_INPUT(0)->payload->token, &submission);
    bool ready = false;
    if (status->code == AMA_OK) { *status = ama_native_event_ready(&submission, &ready); }
    if (status->code == AMA_OK) { results->AMA_OUTPUT(0)->payload->value = ready ? 1 : 0; }
    ama_native_event_submission_drop(&submission);
}
#include "ama_n_01e818c7085dcd0bf0eae8737eaf6e1d2c8228cbbf230029ff59fa8e039b1e6a.end.h"
