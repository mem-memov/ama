/** @file
 * @brief Allocate fixed callback cells before publishing a native service owner.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * separates native storage from Ama identities; reserved cells make callback
 * completion publication allocation-free.
 */
#include <stdlib.h>
#include "native/events/internal.h"
AmaStatus ama_native_event_service_create(size_t capacity, AmaNativeEventKeepalive *keepalive, AmaNativeEventService **output) {
    if (output == NULL || *output != NULL || capacity == 0
        || (keepalive != NULL && ((keepalive->resource == NULL) != (keepalive->release == NULL)))) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native event service creation");
    }
    if (capacity > SIZE_MAX / sizeof(AmaNativeEventCell)) {
        return AMA_NATIVE_EVENT_ERROR(AMA_LIMIT_EXCEEDED, "Native event capacity exceeds storage");
    }
    AmaNativeEventService *service = malloc(sizeof(*service));
    if (service == NULL) { return AMA_NATIVE_EVENT_ERROR(AMA_OUT_OF_MEMORY, "Cannot allocate native event service"); }
    AmaNativeEventCell *cells = malloc(capacity * sizeof(*cells));
    if (cells == NULL) {
        free(service);
        return AMA_NATIVE_EVENT_ERROR(AMA_OUT_OF_MEMORY, "Cannot allocate native event cells");
    }
    for (size_t index = 0; index < capacity; index++) {
        cells[index] = (AmaNativeEventCell){.next = index + 1 < capacity ? index + 1 : AMA_NATIVE_EVENT_NONE,
            .previous = AMA_NATIVE_EVENT_NONE};
    }
    *service = (AmaNativeEventService){.references = 1, .capacity = capacity, .free = 0,
        .first = AMA_NATIVE_EVENT_NONE, .last = AMA_NATIVE_EVENT_NONE, .cells = cells};
    atomic_init(&service->locked, false);
    AmaStatus status = ama_native_event_identity(&service->identity);
    if (status.code != AMA_OK) { free(cells); free(service); return status; }
    if (keepalive != NULL) {
        service->keepalive = *keepalive;
        *keepalive = (AmaNativeEventKeepalive){0};
    }
    *output = service;
    return ama_status_success();
}
