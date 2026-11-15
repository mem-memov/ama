/** @file
 * @brief Execute the explicit arm operation on a retained native service.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * allows native records only; Ama state is accessed solely through these generated borrows.
 */
#include "ama_n_064ab8a1cc1b9a9e8c7de2170765fd12f42ff2785ec8b7c20b83b4a3c5c3643f.interface.h"
#include "ama_n_064ab8a1cc1b9a9e8c7de2170765fd12f42ff2785ec8b7c20b83b4a3c5c3643f.begin.h"
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
/** The manual notification source has no foreign start action; completion is explicit. */
static void AMA_NATIVE(pending)(const AmaNativeEventSubmission *submission, void *resource) { (void)submission; (void)resource; }
/** Execute one bounded native operation; every temporary owner is released before return. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_b80d1a68dbc4fb9c6bb338e7a9b54abb7a6763165fe977559d14c03221b4be63 *parameters,
    ama_t_42dbf36c630d6cb001442f3ec699364e7a199ed6f6685d6cf4100928ddf71788 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services;
    if (state->lease.resource == NULL) { *status = ama_status_make(AMA_INVALID_STATE, NULL, 0); return; }
    AmaNativeEventSubmission submission = {0};
    *status = ama_native_event_service_callback(state->lease.resource, parameters->AMA_INPUT(0)->payload->token, &submission);
    if (status->code == AMA_OK) { *status = ama_native_event_service_arm(state->lease.resource, &submission, AMA_NATIVE(pending), NULL); }
    ama_native_event_submission_drop(&submission);
}
#include "ama_n_064ab8a1cc1b9a9e8c7de2170765fd12f42ff2785ec8b7c20b83b4a3c5c3643f.end.h"
