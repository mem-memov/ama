/**
 * @file
 * @brief Reserve the next clone child and preserve failure-unwind reachability.
 *
 * [COPY-05](../../../../spec/language/values-and-references.md#copy-05)
 * requires fresh identities for the source ownership tree.
 * Source protection precedes this phase; install partial reservations before returning failure.
 */
#include "internal.h"

AmaStatus ama_clone_descend(AmaObjectOperation *operation, AmaObjectWork **cursor) {
    AmaObjectWork *work = *cursor;
    uint64_t type_id;
    const AmaRecipe *recipe;
    AmaHandle *destination = ama_object_child(work, work->next_child, &type_id, &recipe);
    AmaPoolCell *original = NULL;
    (void)ama_pool_lookup(&work->type->pool, work->source, true, &original);
    AmaObjectWork source_work = {.type = work->type, .cell = original};
    AmaHandle child = *ama_object_child(&source_work, work->next_child, &type_id, &recipe);
    AmaObjectWork *child_work = NULL;
    AmaStatus status = ama_clone_reserve(operation, child, work, NULL, (AmaHandle){0}, &child_work);
    if (child_work != NULL) {
        *destination = child_work->handle;
        *cursor = child_work;
    }
    return status;
}
