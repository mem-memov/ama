/** @file
 * @brief Retain exactly one completion in observed acceptance order without allocation.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * carries immutable tokens unchanged; [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * keeps synchronous or racing completions observable until the executor consumes them.
 */
#include <assert.h>
#include "native/events/internal.h"
AmaNativeEventPostResult ama_native_event_complete(void *state, AmaNativeEventToken token,
    int64_t code, AmaNativeEventBuffer *buffer) {
    AmaNativeEventService *service = state;
    ama_native_event_lock(service);
    AmaNativeEventPostResult result;
    bool notify = false;
    AmaNativeEventCell *cell = ama_native_event_cell(service, token);
    if (service->closed) { result = AMA_EVENT_CLOSED; }
    else if (cell == NULL) { result = AMA_EVENT_STALE; }
    else if (cell->state == AMA_EVENT_COMPLETED) { result = AMA_EVENT_DUPLICATE; }
    else if (cell->state != AMA_EVENT_ARMED) { result = AMA_EVENT_UNARMED; }
    else if (!ama_native_event_buffer_valid(service, buffer)) { result = AMA_EVENT_INVALID_BUFFER; }
    else {
        cell->code = code;
        if (buffer != NULL) {
            cell->buffer = *buffer;
            *buffer = (AmaNativeEventBuffer){0};
        }
        assert(service->armed != 0);
        service->armed--;
        cell->state = AMA_EVENT_COMPLETED;
        cell->previous = service->last;
        cell->next = AMA_NATIVE_EVENT_NONE;
        if (service->last == AMA_NATIVE_EVENT_NONE) { service->first = token.slot; }
        else { service->cells[service->last].next = token.slot; }
        service->last = token.slot;
        if (!service->notified) {
            service->notified = true;
            notify = true;
        }
        result = AMA_EVENT_QUEUED;
    }
    ama_native_event_unlock(service);
    if (notify) { ama_native_event_notify(service); }
    return result;
}
AmaStatus ama_native_event_check(void *state, AmaNativeEventToken token, bool *ready) {
    if (ready == NULL) { return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Missing native readiness receiver"); }
    AmaNativeEventService *service = state;
    ama_native_event_lock(service);
    AmaNativeEventCell *cell = ama_native_event_cell(service, token);
    bool live = cell != NULL;
    if (live) { *ready = cell->state == AMA_EVENT_COMPLETED; }
    ama_native_event_unlock(service);
    return live ? ama_status_success() : AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native completion token is no longer live");
}
