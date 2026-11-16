/** @file
 * @brief Serialize finite native channel metadata without locking an Ama executor or invoking callbacks.
 * [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05) requires
 * independent synchronization for shared native services.
 */
#include <threads.h>
#include "../internal.h"

/** Both endpoint owners retain service lifetime before contending for this gate. Component contract: [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_lock(AmaNativeChannel *channel) {
    while (atomic_exchange_explicit(&channel->locked, true, memory_order_acquire)) { thrd_yield(); }
}
/** Publish queue/closure changes to the opposite endpoint. Component contract: [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05). */
void ama_channel_unlock(AmaNativeChannel *channel) {
    atomic_store_explicit(&channel->locked, false, memory_order_release);
}
