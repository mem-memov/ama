/** @file
 * @brief Declare a deliberately shared C-only observer service for nested activation tests.
 * [EMBED-06](../../../../spec/implementation/c-library.md#embed-06) permits
 * compatible guarded support declarations; no Ama value or context pointer is transported here.
 */
#ifndef AMA_NESTED_PROBE_V1_H
#define AMA_NESTED_PROBE_V1_H
#include <stdint.h>
/** Distinguish actual helper activations from opaque native resource acquisition/retirement. */
enum { AMA_NESTED_HELPER_ENTER=1, AMA_NESTED_HELPER_LEAVE,
    AMA_NESTED_RESOURCE_CREATE, AMA_NESTED_RESOURCE_DESTROY };
/** Synchronous C observer. It retains no borrow and attempts no successful Ama execution. */
typedef void (*AmaNestedProbeV1Callback)(uint32_t);
/** Set/clear the explicit host observer before/after the corresponding native activity. */
void ama_nested_probe_v1_set(AmaNestedProbeV1Callback);
/** Notify the current C observer synchronously; no callback is retained on a C stack. */
void ama_nested_probe_v1_event(uint32_t);
#endif
