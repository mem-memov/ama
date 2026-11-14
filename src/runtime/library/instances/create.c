/** @file
 * @brief Construct one explicitly selected exported root at its final pool address.
 * [EMBED-02](../../../../spec/implementation/c-library.md#embed-02) distinguishes
 * construction from entry and makes admitted construction failures context-terminal.
 */
#include "../internal.h"
AmaLibraryResult ama_library_instance_create(AmaLibraryService *service, AmaLibraryContext context,
    uint64_t exported, AmaLibraryInstance *output) {
    if (service == NULL || !service->initialized || output == NULL
        || output->owner != 0 || output->context != 0 || output->identity != 0) {
        return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    const AmaLibraryExport *selected = NULL;
    for (size_t i = 0; i < service->export_count; i++) {
        if (service->exports[i].identity == exported) { selected = &service->exports[i]; break; }
    }
    if (selected == NULL) { return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0)); }
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(service, context, &access);
    if (result.code != AMA_LIBRARY_OK) { return result; }
    if (access.view.status.code != AMA_OK) {
        result = ama_library_result(AMA_LIBRARY_REFUSED, access.view.status);
    } else if (ama_world_type(access.view.world, selected->type) == NULL) {
        result = ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_TYPE_MISMATCH, NULL, 0));
    } else if (access.record->exhausted) {
        result = ama_library_fail(service, &access, ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0));
    } else {
        AmaLibraryRoot *root = service->allocator.allocate(service->allocator.context, sizeof(*root));
        if (root == NULL) { result = ama_library_fail(service, &access, ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0)); }
        else {
            *root = (AmaLibraryRoot){.identity=access.record->next_instance,.exported=exported};
            if (access.record->next_instance == UINT64_MAX) { access.record->exhausted = true; }
            else { access.record->next_instance++; }
            AmaStatus status = ama_object_create(access.view.world, selected->type, NULL, &root->handle);
            if (status.code != AMA_OK) {
                service->allocator.release(service->allocator.context, root);
                result = ama_library_fail(service, &access, status);
            } else {
                root->next = access.record->roots;
                access.record->roots = root;
                *output = (AmaLibraryInstance){(uintptr_t)service,context.identity,root->identity};
                result = ama_library_result(AMA_LIBRARY_OK, ama_status_success());
            }
        }
    }
    ama_library_leave(&access);
    return result;
}
