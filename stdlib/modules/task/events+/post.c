/** @file
 * @brief Execute the explicit post operation on a retained native service.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * allows native records only; Ama state is accessed solely through these generated borrows.
 */
#include "ama_n_0b8e02d03ade380f1a28d5ee97c77e75930ec926372a802d4ac8559e40956422.interface.h"
#include "ama_n_0b8e02d03ade380f1a28d5ee97c77e75930ec926372a802d4ac8559e40956422.begin.h"
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
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_31499f6c2364672483cc1a439cfa858fdabfddb635461af523e8639f9bc2e216 *parameters,
    ama_t_89b3ee82195ec9f658f1a723a204fe2ebe368fae36bba5a4d053ee180ee2d838 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    size_t code = parameters->AMA_INPUT(1)->payload->value;
    if (code > INT64_MAX) { *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); return; }
    AmaByteView bytes;
    *status = ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_view(parameters->AMA_INPUT(2)->payload, &bytes);
    AmaNativeEventSubmission submission = {0};
    AmaNativeEventBuffer buffer = {0};
    if (status->code == AMA_OK) { *status = ama_native_event_service_callback(state->lease.resource, parameters->AMA_INPUT(0)->payload->token, &submission); }
    if (status->code == AMA_OK) { *status = ama_native_event_buffer_copy(&submission, bytes, &buffer); }
    if (status->code == AMA_OK) {
        AmaNativeEventPostResult posted = ama_native_event_post(&submission, (int64_t)code, &buffer);
        if (posted == AMA_EVENT_UNARMED || posted == AMA_EVENT_INVALID_BUFFER) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    }
    ama_native_event_buffer_drop(&buffer);
    ama_native_event_submission_drop(&submission);
}
#include "ama_n_0b8e02d03ade380f1a28d5ee97c77e75930ec926372a802d4ac8559e40956422.end.h"
