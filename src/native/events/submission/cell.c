/** @file
 * @brief Authenticate a native locator with its complete service/submission identity.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * rejects old tokens even when a native record locator is reused.
 */
#include "native/events/internal.h"
AmaNativeEventCell *ama_native_event_cell(AmaNativeEventService *service, AmaNativeEventToken token) {
    if (service->closed || token.domain != service->identity.domain || token.service != service->identity.service
        || token.generation == 0 || token.slot >= service->capacity) { return NULL; }
    AmaNativeEventCell *cell = &service->cells[token.slot];
    return cell->state != AMA_EVENT_FREE && cell->generation == token.generation ? cell : NULL;
}
void ama_native_event_recycle(AmaNativeEventService *service, size_t index) {
    service->cells[index] = (AmaNativeEventCell){.next = service->free, .previous = AMA_NATIVE_EVENT_NONE};
    service->free = index;
}
