/** @file
 * @brief Validate exact native event post admission before an external provider installs a wait.
 * [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * arms before submission; [ASYNC-09](../../../../spec/language/async-and-parallelism.md#async-09)
 * lets retired/closed consumers acknowledge without recreating a wait.
 */
#include "native/events/internal.h"

/** Full authentication excludes recycled locators and distinguishes prepared from armed state. Component contract: [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06), [ASYNC-09](../../../../spec/language/async-and-parallelism.md#async-09). */
AmaNativeEventAcceptance ama_native_event_accept(void *state, AmaNativeEventToken token) {
    AmaNativeEventService *service = state;
    ama_native_event_lock(service);
    AmaNativeEventCell *cell = ama_native_event_cell(service, token);
    AmaNativeEventAcceptance result = AMA_EVENT_TOKEN_RETIRED;
    if (service->closed) { result = AMA_EVENT_SOURCE_CLOSED; }
    else if (cell != NULL) {
        result = cell->state == AMA_EVENT_ARMED ? AMA_EVENT_ACCEPTING
            : cell->state == AMA_EVENT_COMPLETED ? AMA_EVENT_ALREADY_COMPLETED
            : cell->state == AMA_EVENT_PREPARED ? AMA_EVENT_NOT_ARMED : AMA_EVENT_INVALID_SOURCE;
    }
    ama_native_event_unlock(service);
    return result;
}
