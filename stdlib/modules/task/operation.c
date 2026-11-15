/** @file
 * @brief Bind a native registration to explicit Ama ownership and retire it before releasing callback storage.
 * [ASYNC-09](../../../spec/language/async-and-parallelism.md#async-09)
 * permits native acknowledgments to outlive the task without reviving its wait.
 */
#include "ama_n_70a5cd8fc987052849f450f4b33d25da449963dd4b50c7bc4b017092c096d67d.interface.h"
#include "ama_n_70a5cd8fc987052849f450f4b33d25da449963dd4b50c7bc4b017092c096d67d.begin.h"
#include "native/events/assembly.h"
/** Default construction owns no native operation and is safe to destroy immediately. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    *state = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Unlink the exact full token before dropping this owner's native service credit. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    AmaNativeEventSubmission submission = state->submission;
    state->submission = (AmaNativeEventSubmission){0};
    AmaStatus status = ama_status_success();
    if (submission.state != NULL) {
        bool retired = false;
        status = ama_native_event_service_detach(submission.state, submission.token, &retired);
    }
    ama_native_event_submission_drop(&submission);
    return status;
}
/** Borrow the explicitly supplied service only during this callback; retain native rights, never the view. */
static AmaStatus AMA_NATIVE(prepare)(const ama_t_0a13545c236dc688542e3824b23bf3848f38fdb81d6b4c289732f8a82eaa888f *source,
    void *resource) {
    if (source->payload->service == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return ama_native_event_service_prepare(source->payload->service, resource);
}
/** Restart first detaches the previous registration; successful preparation publishes the immutable token last. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_473c8e597f295c997f00cd91e1fac1eaab0758cfb09387fd368af404774b9499 *parameters,
    ama_t_0ec4ace1753d0998a3c4957971f1b035f9c95263c246b89a602da44ea4b66a6d *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)services;
    *status = AMA_NATIVE(destroy)(state);
    if (status->code != AMA_OK) { return; }
    *status = parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage,
        AMA_NATIVE(prepare), &state->submission);
    if (status->code == AMA_OK) { results->AMA_OUTPUT(0)->payload->token = state->submission.token; }
}
#include "ama_n_70a5cd8fc987052849f450f4b33d25da449963dd4b50c7bc4b017092c096d67d.end.h"
