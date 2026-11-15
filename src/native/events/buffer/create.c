/** @file
 * @brief Copy transient source bytes before publishing independent native ownership.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * keeps callbacks independent of source views and their Ama execution scope.
 */
#include <stdlib.h>
#include <string.h>
#include "native/events/buffer/internal.h"
AmaStatus ama_native_event_copy(void *state, AmaByteView source, AmaNativeEventBuffer *output) {
    AmaNativeEventService *service = state;
    if (output == NULL || output->owner != NULL || output->data != NULL
        || output->length != 0 || output->release != NULL || (source.data == NULL && source.length != 0)) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native byte copy");
    }
    if (source.length > SIZE_MAX - sizeof(AmaNativeEventBytes)) {
        return AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native byte allocation exceeds storage");
    }
    AmaNativeEventBytes *bytes = NULL;
    if (source.length != 0) {
        bytes = malloc(sizeof(*bytes) + source.length);
        if (bytes == NULL) { return AMA_NATIVE_EVENT_ERROR(AMA_OUT_OF_MEMORY, "Cannot allocate native event bytes"); }
        bytes->service = service;
        bytes->length = source.length;
        memcpy(bytes->bytes, source.data, source.length);
    }
    ama_native_event_lock(service);
    AmaStatus status = ama_status_success();
    if (service->closed) { status = AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native event service is closed"); }
    else if (bytes != NULL && service->references == SIZE_MAX) {
        status = AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native byte references exhausted");
    } else if (bytes != NULL) { service->references++; }
    ama_native_event_unlock(service);
    if (status.code != AMA_OK) { free(bytes); return status; }
    if (bytes != NULL) {
        *output = (AmaNativeEventBuffer){.data = bytes->bytes, .length = bytes->length,
            .owner = bytes, .release = ama_native_event_buffer_release};
    }
    return ama_status_success();
}
