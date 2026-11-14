/** @file
 * @brief Invalidate public context membership before quiescent native cleanup.
 * [EMBED-02](../../../../spec/implementation/c-library.md#embed-02) makes
 * busy rejection effect-free; cleanup errors cannot resurrect a consumed identity.
 */
#include "../internal.h"
AmaLibraryResult ama_library_context_dispose(AmaLibraryService *service, AmaLibraryContext *context) {
    if (context == NULL) { return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0)); }
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(service, *context, &access);
    if (result.code != AMA_LIBRARY_OK) { return result; }
    AmaLibraryContextRecord *record = access.record;
    ama_library_lock(service);
    AmaLibraryContextRecord **link = &service->first;
    while (*link != record) { link = &(*link)->next; }
    *link = record->next;
    ama_library_unlock(service);
    uint64_t identity = context->identity;
    *context = (AmaLibraryContext){0};
    ama_library_leave(&access);
    /* The service exclusively owns its core registry. Removed host membership
     * prevents new admission between releasing this lease and core invalidation.
     * Native cleanup therefore runs without the host metadata gate held.
     */
    bool admitted = false;
    AmaStatus status = ama_context_dispose_admitted(&service->contexts, identity, &admitted);
    if (!admitted) {
        ama_library_lock(service);
        record->next = service->first;
        service->first = record;
        ama_library_unlock(service);
        *context = (AmaLibraryContext){(uintptr_t)service, identity};
        return ama_library_result(status.code == AMA_BUSY ? AMA_LIBRARY_BUSY : AMA_LIBRARY_REFUSED, status);
    }
    ama_library_forget_roots(service, record);
    service->allocator.release(service->allocator.context, record);
    return ama_library_result(status.code == AMA_OK ? AMA_LIBRARY_OK : AMA_LIBRARY_TECHNICAL_FAILURE, status);
}
