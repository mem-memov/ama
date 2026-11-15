/** @file
 * @brief Own one POSIX process-private notification latch for a native completion service.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * requires blocking without losing queued or concurrently arriving notifications;
 * the latch outlives the owner until all native callback/byte leases retire.
 */
#include <assert.h>
#include "native/events/internal.h"
AmaStatus ama_native_event_wake_initialize(AmaNativeEventService *service) {
    if (sem_init(&service->wake, 0, 0) != 0) {
        return AMA_NATIVE_EVENT_ERROR(AMA_NATIVE_FAILURE, "Cannot initialize native completion wake");
    }
    service->notified = false;
    return ama_status_success();
}
void ama_native_event_wake_destroy(AmaNativeEventService *service) {
    /* Owner operations serialize wait/dispose; final callback credits have also
     * retired. No waiter can remain on this live initialized semaphore. */
    int destroyed = sem_destroy(&service->wake);
    assert(destroyed == 0);
    (void)destroyed;
}
