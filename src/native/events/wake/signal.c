/** @file
 * @brief Publish one reserved wake credit without executing Ama or holding its metadata gate.
 * [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * retains data while [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * requires the corresponding notification to survive a check-to-sleep race.
 */
#include <assert.h>
#include "native/events/internal.h"
void ama_native_event_notify(AmaNativeEventService *service) {
    /* Only false->true reserves a post. The sole consumer clears notified only
     * after consuming that credit. Thus at most one credit exists/in flight;
     * a valid live semaphore cannot overflow. Its callback lease keeps it live. */
    int signaled = sem_post(&service->wake);
    assert(signaled == 0);
    (void)signaled;
}
