/** @file
 * @brief Publish a host context only after all context-local constants initialize.
 * [EMBED-02](../../../../spec/implementation/c-library.md#embed-02) prohibits
 * implicit executable roots and requires failed startup to return its status directly.
 */
#include "../internal.h"
AmaLibraryResult ama_library_context_create(AmaLibraryService *service, AmaLibraryContext *output) {
    if (service == NULL || !service->initialized || output == NULL || output->owner != 0 || output->identity != 0) {
        return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    AmaLibraryContextRecord *record = service->allocator.allocate(service->allocator.context, sizeof(*record));
    if (record == NULL) { return ama_library_result(AMA_LIBRARY_TECHNICAL_FAILURE, ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0)); }
    *record = (AmaLibraryContextRecord){.next_instance=1};
    AmaStatus status = ama_context_create(&service->contexts, service->definition, &record->identity);
    if (status.code != AMA_OK) {
        service->allocator.release(service->allocator.context, record);
        return ama_library_result(AMA_LIBRARY_TECHNICAL_FAILURE, status);
    }
    AmaLibraryContext published = {atomic_load_explicit(&service->owner, memory_order_acquire), record->identity};
    ama_library_lock(service);
    record->next = service->first;
    service->first = record;
    ama_library_unlock(service);
    *output = published;
    return ama_library_result(AMA_LIBRARY_OK, ama_status_success());
}
