/** @file
 * @brief Validate and retire native byte owners through their matching creator implementation.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * gives queued results native ownership instead of borrowing mutable Ama storage.
 */
#include <stdlib.h>
#include "native/events/buffer/internal.h"
bool ama_native_event_buffer_valid(AmaNativeEventService *service, const AmaNativeEventBuffer *buffer) {
    if (buffer == NULL) { return true; }
    if (buffer->owner == NULL) { return buffer->data == NULL && buffer->length == 0 && buffer->release == NULL; }
    if (buffer->release != ama_native_event_buffer_release) { return false; }
    const AmaNativeEventBytes *bytes = buffer->owner;
    return bytes->service == service && buffer->data == bytes->bytes && buffer->length == bytes->length;
}
void ama_native_event_buffer_release(void *owner) {
    AmaNativeEventBytes *bytes = owner;
    AmaNativeEventService *service = bytes->service;
    free(bytes);
    ama_native_event_release(service);
}
