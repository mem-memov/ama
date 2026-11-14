/** @file
 * @brief Resolve exact export-bound instance controls under one context executor lease.
 * [EMBED-03](../../../../spec/implementation/c-library.md#embed-03) rejects
 * foreign, stale, unavailable and wrong-export targets before data construction.
 */
#include "../internal.h"
AmaLibraryResult ama_library_select(AmaLibraryService *service, AmaLibraryContext context,
    AmaLibraryInstance instance, uint64_t exported, AmaAccessMode mode,
    AmaLibraryAccess *access, AmaHandle *output) {
    if (service==NULL || instance.owner!=(uintptr_t)service || instance.context!=context.identity
        || instance.identity==0 || access==NULL || output==NULL
        || mode<AMA_ACCESS_ENTRY || mode>AMA_ACCESS_READ) {
        return ama_library_result(AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0));
    }
    AmaLibraryResult result=ama_library_enter(service,context,access);
    if (result.code!=AMA_LIBRARY_OK) { return result; }
    AmaLibraryRoot *root=access->record->roots;
    while (root!=NULL && root->identity!=instance.identity) { root=root->next; }
    if (root==NULL || root->exported!=exported) {
        result=ama_library_result(AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0));
    } else if (access->view.status.code!=AMA_OK) {
        result=ama_library_result(AMA_LIBRARY_REFUSED,access->view.status);
    } else if (access->view.world->operation_active) {
        result=ama_library_result(AMA_LIBRARY_BUSY,ama_status_make(AMA_BUSY,NULL,0));
    } else {
        AmaPoolCell *cell=NULL;
        AmaStatus status=ama_program_cell(access->view.program,root->handle,&cell);
        if (status.code==AMA_OK) { status=ama_instance_access(&cell->metadata,mode); }
        if (status.code==AMA_OK) { *output=root->handle; return result; }
        result=ama_library_result(status.code==AMA_BUSY ? AMA_LIBRARY_BUSY : AMA_LIBRARY_REFUSED,status);
    }
    ama_library_leave(access);
    return result;
}
