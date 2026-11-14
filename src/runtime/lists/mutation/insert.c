/**
 * @file
 * @brief Prepare one owned value or reference before inserting its handle.
 *
 * [LIST-10](../../../../spec/language/lists.md#list-10) reserves membership metadata before native construction so publication cannot allocate.
 * [LIFE-04](../../../../spec/implementation/native-interface.md#life-04)
 * The storage guard supplies validated bounds and protected sources. Commit shifts membership indices, not native payloads.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "runtime/lists/membership/internal.h"

/** Require validated position and guarded mutation inputs. Prepare/publish one value before shifting handles; failure before commit preserves contents, though capacity may grow. */
AmaStatus ama_list_insert(AmaListMutation *mutation, size_t position, AmaHandle source) {
    AmaHandleList *list = mutation->list;
    if (list->count == SIZE_MAX || (mutation->owning && mutation->parent->metadata.lifetime.live_children == SIZE_MAX)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    AmaStatus status = ama_membership_prepare(list, list->count + 1);
    if (status.code != AMA_OK) { return status; }
    AmaHandle element = source;
    if (mutation->owning) {
        AmaRecipe recipe = {.constructor = source.generation == 0 ? AMA_CONSTRUCT_DEFAULT : AMA_CONSTRUCT_CLONE, .source = source};
        status = ama_object_build(&mutation->objects, mutation->type, &recipe, &element);
        if (status.code != AMA_OK) { return status; }
        for (AmaObjectWork *work = mutation->objects.first; work != NULL; work = work->next) {
            if (work->parent == NULL) { work->cell->owner = mutation->owner; }
            AmaServiceState service = work->cell->metadata.service;
            (void)ama_pool_publish(&work->type->pool, work->handle);
            if (work->source.generation != 0) { work->cell->metadata.service = service; }
        }
        mutation->parent->metadata.lifetime.live_children++;
        mutation->parent->metadata.lifetime.child_capacity++;
    }
    AmaMembership identity;
    /* Preparation reserved the exact slot and order extent before native work;
     * this publication cannot allocate, refuse, or call application code. */
    (void)ama_membership_insert(list, position, element, &identity);
    return ama_status_success();
}
