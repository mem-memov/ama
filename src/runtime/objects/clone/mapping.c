/**
 * @file
 * @brief Remap declared reference fields through the complete operation-scoped copy map.
 *
 * [COPY-04](../../../../spec/language/values-and-references.md#copy-04)
 * keeps external references unchanged and remaps only the selected ownership boundary.
 * Empty, stale external and independently copied boundaries remain distinct.
 */
#include "internal.h"

/** Apply the selected boundary's injective source map, or preserve an external
 * handle byte-for-byte. final_root substitutes only the boundary root identity.
 */
AmaHandle ama_clone_reference_map(const void *context, AmaHandle source) {
    const AmaCloneMap *map = context;
    const AmaObjectOperation *operation = map->operation;
    if (source.generation == 0) { return source; }
    for (const AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
        if (work->clone_root == map->boundary && ama_same_handle(work->source, source)) {
            return work == map->boundary && map->final_root.generation != 0 ? map->final_root : work->identity;
        }
    }
    return source;
}

/** Rewrite only declared nonowning fields in one complete copy boundary. No
 * allocation/native execution occurs; all destination identities must exist.
 * Other boundaries in the same operation and the source tree remain unchanged.
 */
AmaStatus ama_object_fix_references(AmaObjectOperation *operation, AmaObjectWork *boundary,
    AmaHandle final_root, bool copy_continuation) {
    const AmaCloneMap map = {operation, boundary, final_root};
    for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
        if (work->clone_root != boundary) { continue; }
        void *storage = ama_pool_payload(&work->type->pool, work->cell);
        for (size_t field = 0; field < work->type->field_count; field++) {
            if (ama_field_owns(work->type->fields[field].kind)) { continue; }
            size_t count = ama_object_field_count(work->type, storage, field);
            for (size_t index = 0; index < count; index++) {
                AmaHandle *handle = ama_object_field_handle(work->type, storage, field, index);
                *handle = ama_clone_reference_map(&map, *handle);
            }
        }
        AmaStatus status = ama_clone_control_map(operation, work, &map, copy_continuation);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_success();
}
