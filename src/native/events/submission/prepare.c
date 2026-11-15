/** @file
 * @brief Reserve immutable native submission identity before any foreign operation is armed.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * forbids token reuse; [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * needs a final token available before the executor publishes its Ama registration.
 */
#include "native/events/internal.h"
AmaStatus ama_native_event_service_prepare(AmaNativeEventService *service, AmaNativeEventSubmission *output) {
    if (service == NULL || output == NULL || output->state != NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native event reservation");
    }
    ama_native_event_lock(service);
    AmaStatus status = ama_status_success();
    uint64_t next;
    if (service->closed) { status = AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native event service is closed"); }
    else if (service->free == AMA_NATIVE_EVENT_NONE) { status = AMA_NATIVE_EVENT_ERROR(AMA_BUSY, "Native event capacity is full"); }
    else if (service->references == SIZE_MAX || !ama_native_event_sequence(service->generation, UINT64_MAX, &next)) {
        status = AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native submission identity or references exhausted");
    } else {
        size_t index = service->free;
        service->free = service->cells[index].next;
        service->cells[index] = (AmaNativeEventCell){.state = AMA_EVENT_PREPARED, .generation = next,
            .next = AMA_NATIVE_EVENT_NONE, .previous = AMA_NATIVE_EVENT_NONE};
        service->generation = next;
        service->references++;
        AmaNativeEventToken token = service->identity;
        token.generation = next;
        token.slot = index;
        *output = (AmaNativeEventSubmission){.state = service, .callbacks = ama_native_event_callbacks(), .token = token};
    }
    ama_native_event_unlock(service);
    return status;
}

/** Preserve the supplied identity while granting only native callback resource rights. */
AmaStatus ama_native_event_service_callback(AmaNativeEventService *service, AmaNativeEventToken token,
    AmaNativeEventSubmission *output) {
    if (service == NULL || output == NULL || output->state != NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native callback binding");
    }
    if (!ama_native_event_retain(service)) {
        return AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native callback references exhausted");
    }
    *output = (AmaNativeEventSubmission){service, ama_native_event_callbacks(), token};
    return ama_status_success();
}
