/**
 * @file
 * @brief Drive private iterative clone preparation, fixups and prefix rollback.
 *
 * [COPY-02](../../../../spec/language/values-and-references.md#copy-02)
 * requires complete preparation before destructive publication.
 * Caller holds the world guard and complete source protections; output changes only on success.
 */
#include "internal.h"

/** Prepare one copy boundary under existing source protections. Each reservation
 * is owned by operation until publication or reverse-prefix cleanup. final_root
 * redirects root/self references for replacement without changing source storage.
 */
AmaStatus ama_object_clone_prepare(AmaObjectOperation *operation, AmaHandle source, AmaHandle final_root,
    bool continuation, void *root_payload, AmaHandle *output) {
    AmaObjectWork *work = NULL;
    AmaStatus status = ama_clone_reserve(operation, source, NULL, root_payload, &work);
    AmaObjectWork *root = work;
    while (status.code == AMA_OK) {
        if (work->next_child < work->cell->metadata.lifetime.child_capacity) {
            status = ama_clone_descend(operation, &work);
        } else {
            status = ama_object_clone_payload(operation, work, continuation);
            if (status.code != AMA_OK || work->parent == NULL) { break; }
            work = work->parent;
            work->next_child++;
            status = ama_lifetime_child_ready(&work->cell->metadata.lifetime);
        }
    }
    if (status.code != AMA_OK) {
        ama_object_report(operation->world, status);
        while (work != NULL) {
            status = ama_object_cleanup(operation->world, work->handle, status);
            work = work->parent;
        }
        return status;
    }
    ama_object_fix_references(operation, root, final_root);
    *output = root->handle;
    return ama_status_success();
}
