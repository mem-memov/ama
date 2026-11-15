/** @file
 * @brief Publish native armed state before a submit routine can synchronously complete.
 * [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * requires retained completion across the submit/recheck interval; this routine
 * invokes foreign C only after releasing the native metadata gate.
 */
#include "native/events/internal.h"
AmaStatus ama_native_event_service_arm(AmaNativeEventService *service, const AmaNativeEventSubmission *submission,
    AmaNativeEventStart start, void *native_resource) {
    if (service == NULL || submission == NULL || submission->state != service
        || submission->callbacks != ama_native_event_callbacks() || start == NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native event arm");
    }
    ama_native_event_lock(service);
    AmaNativeEventCell *cell = ama_native_event_cell(service, submission->token);
    bool armed = cell != NULL && cell->state == AMA_EVENT_PREPARED;
    if (armed) { cell->state = AMA_EVENT_ARMED; }
    ama_native_event_unlock(service);
    if (!armed) { return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native submission cannot be armed"); }
    start(submission, native_resource);
    return ama_status_success();
}
AmaStatus ama_native_event_service_discard(AmaNativeEventService *service, AmaNativeEventToken token) {
    if (service == NULL) { return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Empty native event owner"); }
    ama_native_event_lock(service);
    AmaNativeEventCell *cell = ama_native_event_cell(service, token);
    bool discarded = cell != NULL && cell->state == AMA_EVENT_PREPARED;
    if (discarded) { ama_native_event_recycle(service, token.slot); }
    ama_native_event_unlock(service);
    return discarded ? ama_status_success() : AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native reservation cannot be discarded");
}
