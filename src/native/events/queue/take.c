/** @file
 * @brief Move the oldest retained native result to the executor and recycle only its locator.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * keeps import separate from callback publication; old tokens cannot name a reused cell.
 */
#include "native/events/internal.h"
AmaStatus ama_native_event_service_take(AmaNativeEventService *service, AmaNativeEventCompletion *output, bool *available) {
    if (service == NULL || output == NULL || available == NULL || output->buffer.owner != NULL
        || output->buffer.data != NULL || output->buffer.length != 0 || output->buffer.release != NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native completion receiver");
    }
    ama_native_event_lock(service);
    if (service->closed) {
        ama_native_event_unlock(service);
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native event service is closed");
    }
    size_t index = service->first;
    *available = index != AMA_NATIVE_EVENT_NONE;
    if (*available) {
        AmaNativeEventCell *cell = &service->cells[index];
        service->first = cell->next;
        if (cell->next == AMA_NATIVE_EVENT_NONE) { service->last = AMA_NATIVE_EVENT_NONE; }
        else { service->cells[cell->next].previous = AMA_NATIVE_EVENT_NONE; }
        AmaNativeEventToken token = service->identity;
        token.generation = cell->generation;
        token.slot = index;
        *output = (AmaNativeEventCompletion){.token = token, .code = cell->code, .buffer = cell->buffer};
        ama_native_event_recycle(service, index);
    }
    ama_native_event_unlock(service);
    return ama_status_success();
}
