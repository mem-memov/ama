/** @file
 * @brief Retire delivery rights at an explicit task boundary while the owner and callbacks retain their own native leases.
 * [ASYNC-08](../../../../spec/language/async-and-parallelism.md#async-08)
 * requires cooperative cleanup; [ASYNC-09](../../../../spec/language/async-and-parallelism.md#async-09)
 * keeps outstanding callback resources alive after detachment.
 */
#include "ama_n_340d675534d11c1132bdecb44ce4b585457f062f624bcb159e94f8132c15e24c.interface.h"
#include "ama_n_340d675534d11c1132bdecb44ce4b585457f062f624bcb159e94f8132c15e24c.begin.h"
#include "native/events/assembly.h"
/** Default construction owns no external resource. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Generated Ama fields, not this payload hook, own declared data. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Copy only the resource-free native value; no subscription or scheduler enrollment is acquired. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) { *destination = *source; return ama_status_success(); }

/** Borrow only long enough to unlink the immutable full token; do not mutate or retain the Ama payload. */
static AmaStatus AMA_NATIVE(detach)(const ama_t_3211b272d4ffc58b35668036530071d3838391fee0aa5c4208851404f0010b09 *operation,
    void *resource) {
    const AmaNativeEventSubmission *submission = &operation->payload->submission;
    if (submission->state == NULL) { *(bool *)resource = false; return ama_status_success(); }
    return ama_native_event_service_detach(submission->state, submission->token, resource);
}
/** The caller must invalidate its declared visible wait first; this method never resumes the task or waits for a worker. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_fb6f876058bc8f3f16501aa7c56064c475b7256bfbfcd860c328104ca5dd0a12 *parameters,
    ama_t_dcbd7ff331ea98eeb89c85b93783342eec9f5160f027558da93de76a77816d83 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)services;
    bool retired = false;
    *status = parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage, AMA_NATIVE(detach), &retired);
    if (status->code == AMA_OK) { results->AMA_OUTPUT(0)->payload->value = retired; }
}
#include "ama_n_340d675534d11c1132bdecb44ce4b585457f062f624bcb159e94f8132c15e24c.end.h"
