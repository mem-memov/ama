/** @file
 * @brief Bind independent immutable native bytes to their creator service's lifetime.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * permits retained native buffers but forbids retaining borrowed Ama views.
 */
#ifndef AMA_NATIVE_EVENTS_BUFFER_INTERNAL_H
#define AMA_NATIVE_EVENTS_BUFFER_INTERNAL_H
#include "native/events/internal.h"
/** One native allocation; the service reference also retains its optional code lease. */
typedef struct {
    AmaNativeEventService *service;
    size_t length;
    unsigned char bytes[];
} AmaNativeEventBytes;
#endif
