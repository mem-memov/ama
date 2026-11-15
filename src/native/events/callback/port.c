/** @file
 * @brief Route external C callback operations into the resource's creator image.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * exposes only native queue/resource operations, with no Ama execution function.
 */
#include "native/events/internal.h"
const AmaNativeEventCallbacks *ama_native_event_callbacks(void) {
    static const AmaNativeEventCallbacks callbacks = {
        .retain = ama_native_event_retain, .drop = ama_native_event_release,
        .copy = ama_native_event_copy, .post = ama_native_event_complete, .ready = ama_native_event_check,
        .acceptance = ama_native_event_accept
    };
    return &callbacks;
}
