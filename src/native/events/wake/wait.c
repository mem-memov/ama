/** @file
 * @brief Block the sole native consumer on a retained binary credit, never on a disappearing edge.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * checks retained completions before blocking and reports native idleness without spinning.
 * Visible Ama wait/phase reconciliation remains the scheduler's responsibility.
 */
#include <errno.h>
#include "native/events/internal.h"
AmaStatus ama_native_event_service_wait(AmaNativeEventService *service, bool *ready) {
    if (service == NULL || ready == NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native completion wait");
    }
    for (;;) {
        ama_native_event_lock(service);
        bool closed = service->closed;
        bool queued = service->first != AMA_NATIVE_EVENT_NONE;
        bool pending = service->armed != 0;
        ama_native_event_unlock(service);
        if (closed) { return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native completion service is closed"); }
        if (queued || !pending) { *ready = queued; return ama_status_success(); }
        int received;
        do { received = sem_wait(&service->wake); } while (received != 0 && errno == EINTR);
        if (received != 0) { return AMA_NATIVE_EVENT_ERROR(AMA_IO_FAILURE, "Native completion wait failed"); }
        ama_native_event_lock(service);
        service->notified = false;
        ama_native_event_unlock(service);
        /* A completion racing credit consumption may have observed notified=true
         * and coalesced its post. The next metadata check still observes its data. */
    }
}
