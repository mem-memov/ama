/** @file
 * @brief Serialize only native queue metadata, without acquiring any Ama object.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * allows concurrent callbacks to publish records but not enter the executor.
 */
#include <threads.h>
#include "native/events/internal.h"
void ama_native_event_lock(AmaNativeEventService *service) {
    while (atomic_exchange_explicit(&service->locked, true, memory_order_acquire)) { thrd_yield(); }
}
void ama_native_event_unlock(AmaNativeEventService *service) {
    atomic_store_explicit(&service->locked, false, memory_order_release);
}
