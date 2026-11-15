/** @file
 * @brief Consume one checked root without ever resuming its saved continuation.
 * [EMBED-02](../../../../spec/implementation/c-library.md#embed-02) preserves
 * owner/generation validity and continues context cleanup after destructor failure.
 */
#include "../internal.h"
AmaLibraryResult ama_library_instance_dispose(AmaLibraryService *service, AmaLibraryContext context,
    AmaLibraryInstance *instance) {
    if (instance == NULL || service == NULL || instance->owner != atomic_load_explicit(&service->owner, memory_order_acquire)
        || instance->context != context.identity || instance->identity == 0) {
        return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(service, context, &access);
    if (result.code != AMA_LIBRARY_OK) { return result; }
    AmaLibraryRoot **link = &access.record->roots;
    while (*link != NULL && (*link)->identity != instance->identity) { link = &(*link)->next; }
    if (*link == NULL) { result = ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0)); }
    else {
        AmaLibraryRoot *root = *link;
        bool admitted = false;
        AmaStatus status = ama_object_destroy_admitted(access.view.world, root->handle, &admitted);
        if (!admitted) {
            if (status.code == AMA_BUSY) { result = ama_library_result(AMA_LIBRARY_BUSY, status); }
            else {
                *instance = (AmaLibraryInstance){0};
                result = ama_library_fail(service, &access, status);
            }
        } else {
            *link = root->next;
            *instance = (AmaLibraryInstance){0};
            service->allocator.release(service->allocator.context, root);
            result = status.code == AMA_OK ? ama_library_result(AMA_LIBRARY_OK, status)
                : ama_library_fail(service, &access, status);
        }
    }
    ama_library_leave(&access);
    return result;
}
