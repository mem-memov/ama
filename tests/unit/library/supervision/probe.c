/** @file
 * @brief Implement a scalar-only C observer dependency shared deliberately by test components.
 * [EMBED-08](../../../../spec/implementation/c-library.md#embed-08) requires
 * every synchronous helper to return before suspension; events expose those boundaries.
 */
#include <stdatomic.h>
#include <stddef.h>
#include "probe.h"
/** Host instrumentation only; this service stores no Ama instance, view or program value. */
static _Atomic(AmaNestedProbeV1Callback) observer;
void ama_nested_probe_v1_set(AmaNestedProbeV1Callback callback) {
    atomic_store_explicit(&observer,callback,memory_order_release);
}
void ama_nested_probe_v1_event(uint32_t event) {
    AmaNestedProbeV1Callback callback=atomic_load_explicit(&observer,memory_order_acquire);
    if (callback!=NULL) { callback(event); }
}
