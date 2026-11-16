/**
 * @file
 * @brief Prepare the selected constructor in the destination spare bank before cleanup.
 *
 * [LIFE-04](../../../../spec/implementation/native-interface.md#life-04)
 * Preparation failure preserves the old value; destructive failure terminates the world. Final self references name the retained destination rather than scratch storage.
 */
#include <string.h>
#include "runtime/objects/internal.h"

/** Within an existing operation, select direct clone replacement or one default/literal recipe in the inactive bank. No default-then-overwrite construction and no payload relocation. Component contract: [LIFE-04](../../../../spec/implementation/native-interface.md#life-04). */
AmaStatus ama_object_reconstruct_in(AmaObjectOperation *operation, AmaHandle destination, const AmaRecipe *recipe) {
    if (recipe != NULL && recipe->constructor == AMA_CONSTRUCT_CLONE && recipe->source.generation != 0) {
        return ama_object_replace_in(operation, destination, recipe->source, recipe->continuation);
    }
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_lookup(operation->world, destination, false, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_object_preflight(operation, destination, false, false);
    if (status.code != AMA_OK) { return status; }
    operation->root_identity = destination;
    operation->root_payload = ama_pool_spare_payload(&type->pool, cell);
    memset(operation->root_payload, 0, type->pool.payload_size);
    AmaHandle prepared = {0};
    status = ama_object_build(operation, destination.type_id, recipe, &prepared);
    if (status.code != AMA_OK) { return status; }
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (ama_same_handle(record->handle, destination)) { record->restore = false; }
    }
    status = ama_object_cleanup_keep(operation->world, destination, ama_status_success(), true);
    if (status.code == AMA_OK) { ama_object_replacement_publish(operation, type, cell, destination); }
    else {
        operation->world->failed = true;
        status = ama_object_cleanup(operation->world, prepared, status);
    }
    return status;
}
