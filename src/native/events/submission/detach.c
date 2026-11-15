/** @file
 * @brief Retire one immutable native identity without reclaiming outstanding callback rights.
 * [ASYNC-09](../../../../spec/language/async-and-parallelism.md#async-09)
 * invalidates visible waits before native detachment and keeps native state alive
 * independently of the former Ama slot until final acknowledgment.
 */
#include <assert.h>
#include "native/events/internal.h"
/** Retire one current cell under the gate and release queued bytes afterward; callbacks retain independent rights. */
AmaStatus ama_native_event_service_detach(AmaNativeEventService *service, AmaNativeEventToken token, bool *retired) {
    if (service == NULL || retired == NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native operation detachment");
    }
    AmaNativeEventBuffer buffer = {0};
    ama_native_event_lock(service);
    AmaNativeEventCell *cell = ama_native_event_cell(service, token);
    *retired = cell != NULL;
    if (cell != NULL) {
        if (cell->state == AMA_EVENT_ARMED) {
            assert(service->armed != 0);
            service->armed--;
        } else if (cell->state == AMA_EVENT_COMPLETED) {
            if (cell->previous == AMA_NATIVE_EVENT_NONE) { service->first = cell->next; }
            else { service->cells[cell->previous].next = cell->next; }
            if (cell->next == AMA_NATIVE_EVENT_NONE) { service->last = cell->previous; }
            else { service->cells[cell->next].previous = cell->previous; }
            buffer = cell->buffer;
        } else { assert(cell->state == AMA_EVENT_PREPARED); }
        ama_native_event_recycle(service, token.slot);
    }
    ama_native_event_unlock(service);
    /* Do not clear notified: a credit may already exist or be in flight. Only
     * its sole waiter may clear the binary latch after consuming that credit. */
    ama_native_event_buffer_drop(&buffer);
    return ama_status_success();
}
