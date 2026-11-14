/** @file
 * @brief Own stable explicit C service lifetime and immutable export admission metadata.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) and
 * [EMBED-02](../../../spec/implementation/c-library.md#embed-02) separate a
 * configured service from context creation and from all executable bodies.
 */
#include <string.h>
#include "internal.h"
AmaStatus ama_library_service_init(AmaLibraryService *service, SourceAllocator allocator,
    const AmaContextDefinition *definition, const AmaLibraryExport *exports, size_t count) {
    if (service == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *service = (AmaLibraryService){0};
    if (definition == NULL || exports == NULL || count == 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    for (size_t i = 0; i < count; i++) {
        if (exports[i].identity == 0 || exports[i].type == 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        for (size_t j = 0; j < i; j++) {
            if (exports[j].identity == exports[i].identity) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        }
    }
    *service = (AmaLibraryService){.allocator=allocator, .definition=definition, .exports=exports, .export_count=count};
    atomic_init(&service->locked, false);
    AmaStatus status = ama_context_registry_init(&service->contexts, allocator);
    service->initialized = status.code == AMA_OK;
    return status;
}
AmaStatus ama_library_service_destroy(AmaLibraryService *service) {
    if (service == NULL || !service->initialized) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    ama_library_lock(service);
    bool live = service->first != NULL;
    ama_library_unlock(service);
    if (live) { return ama_status_make(AMA_BUSY, NULL, 0); }
    AmaStatus status = ama_context_registry_destroy(&service->contexts);
    if (!service->contexts.initialized) { service->initialized = false; }
    return status;
}
AmaLibraryResult ama_library_result(AmaLibraryCode code, AmaStatus status) {
    return (AmaLibraryResult){code, ama_status_checked(status)};
}
void ama_library_forget_roots(AmaLibraryService *service, AmaLibraryContextRecord *record) {
    while (record->roots != NULL) {
        AmaLibraryRoot *root = record->roots;
        record->roots = root->next;
        service->allocator.release(service->allocator.context, root);
    }
}
AmaLibraryResult ama_library_fail(AmaLibraryService *service, AmaLibraryAccess *access, AmaStatus status) {
    AmaStatus failed = ama_context_fail(&access->lease, status);
    ama_library_forget_roots(service, access->record);
    return ama_library_result(AMA_LIBRARY_TECHNICAL_FAILURE, failed);
}

AmaLibraryResult ama_library_fail_at(AmaLibraryService *service, AmaLibraryAccess *access,
    AmaStatus status, AmaHandle target, uint64_t operation) {
    access->view.control->current=target;
    AmaPoolCell *cell=NULL;
    AmaStatus found=ama_program_cell(access->view.program,target,&cell);
    if (found.code==AMA_OK) { cell->metadata.service.current_operation=operation; }
    return ama_library_fail(service,access,status);
}
