/** @file
 * @brief Observe retained work without spending an import budget or entering an OS wait.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * bounds collection and checks pending completions before blocking; this snapshot
 * does not remove a record or grant permission to ignore the wait recheck.
 */
#include "native/events/internal.h"
AmaStatus ama_native_event_service_poll(AmaNativeEventService *service, AmaNativeEventReadiness *output) {
    if (service == NULL || output == NULL) {
        return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Invalid native completion observation");
    }
    ama_native_event_lock(service);
    bool closed = service->closed;
    AmaNativeEventReadiness state = {service->first != AMA_NATIVE_EVENT_NONE, service->armed != 0};
    ama_native_event_unlock(service);
    if (closed) { return AMA_NATIVE_EVENT_ERROR(AMA_INVALID_STATE, "Native completion service is closed"); }
    *output = state;
    return ama_status_success();
}
