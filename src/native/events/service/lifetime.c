/** @file
 * @brief Keep native callbacks and byte owners alive independently of their executor context.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * requires retained native resources; [EMBED-06](../../../../spec/implementation/c-library.md#embed-06)
 * lets their supplied code lease survive context destruction until final acknowledgment.
 */
#include <assert.h>
#include <stdlib.h>
#include "native/events/internal.h"
bool ama_native_event_retain(void *state) {
    AmaNativeEventService *service = state;
    ama_native_event_lock(service);
    bool retained = service->references != 0 && service->references != SIZE_MAX;
    if (retained) { service->references++; }
    ama_native_event_unlock(service);
    return retained;
}
void ama_native_event_release(void *state) {
    AmaNativeEventService *service = state;
    ama_native_event_lock(service);
    assert(service->references != 0);
    service->references--;
    bool final = service->references == 0;
    ama_native_event_unlock(service);
    if (final) {
        assert(service->closed && service->cells == NULL);
        AmaNativeEventKeepalive keepalive = service->keepalive;
        ama_native_event_wake_destroy(service);
        free(service);
        if (keepalive.release != NULL) { keepalive.release(keepalive.resource); }
    }
}
void ama_native_event_service_dispose(AmaNativeEventService **owner) {
    if (owner == NULL || *owner == NULL) { return; }
    AmaNativeEventService *service = *owner;
    *owner = NULL;
    ama_native_event_lock(service);
    service->closed = true;
    service->armed = 0;
    AmaNativeEventCell *cells = service->cells;
    service->cells = NULL;
    service->free = service->first = service->last = AMA_NATIVE_EVENT_NONE;
    ama_native_event_unlock(service);
    /* Closed callbacks cannot access cells. The unique owner remains retained
     * throughout this loop, breaking queue-buffer/service cycles before its drop. */
    for (size_t index = 0; index < service->capacity; index++) { ama_native_event_buffer_drop(&cells[index].buffer); }
    free(cells);
    ama_native_event_release(service);
}

/** Bind an owned native method resource without exposing the service's mutable internals. */
AmaStatus ama_native_event_service_lease(AmaNativeEventService *service, AmaNativeEventKeepalive *output) {
    if (service == NULL || output == NULL || output->resource != NULL || output->release != NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native service lease");
    }
    if (!ama_native_event_retain(service)) {
        return AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native service references exhausted");
    }
    *output = (AmaNativeEventKeepalive){service, ama_native_event_release};
    return ama_status_success();
}
