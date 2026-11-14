/** @file
 * @brief Validate owner/context identity before obtaining a private Ama lease.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) rejects stale
 * or foreign control values before dereferencing context or instance storage.
 */
#include <threads.h>
#include "internal.h"
void ama_library_lock(AmaLibraryService *service) {
    while (atomic_exchange_explicit(&service->locked, true, memory_order_acquire)) { thrd_yield(); }
}
void ama_library_unlock(AmaLibraryService *service) {
    atomic_store_explicit(&service->locked, false, memory_order_release);
}
AmaLibraryResult ama_library_enter(AmaLibraryService *service, AmaLibraryContext context, AmaLibraryAccess *access) {
    if (service == NULL || !service->initialized || access == NULL
        || context.owner != (uintptr_t)service || context.identity == 0) {
        return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    ama_library_lock(service);
    AmaLibraryContextRecord *record = service->first;
    while (record != NULL && record->identity != context.identity) { record = record->next; }
    AmaStatus status = record == NULL ? ama_status_make(AMA_INVALID_STATE, NULL, 0)
        : ama_context_enter(&service->contexts, context.identity, &access->lease);
    if (status.code == AMA_OK) { access->record = record; }
    ama_library_unlock(service);
    if (status.code != AMA_OK) {
        return ama_library_result(status.code == AMA_BUSY ? AMA_LIBRARY_BUSY : AMA_LIBRARY_INVALID_REQUEST, status);
    }
    status = ama_context_inspect(&access->lease, &access->view);
    if (status.code != AMA_OK) { ama_library_leave(access); return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, status); }
    return ama_library_result(AMA_LIBRARY_OK, ama_status_success());
}
void ama_library_leave(AmaLibraryAccess *access) {
    (void)ama_context_leave(&access->lease);
    *access = (AmaLibraryAccess){0};
}
