/**
 * @file
 * @brief Prepare one declared field recipe and its handle storage before child construction.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires explicit lifecycle ordering.
 * Earlier fields remain available to exact generated defaults; checked counts bound buffers/ledgers.
 * [LIFE-04](../../../../spec/implementation/native-interface.md#life-04)
 * exposes exact final self identity only within this synchronous resolver scope;
 * every callback outcome clears that borrowed authority before later work.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

AmaStatus ama_construction_prepare_field(AmaObjectOperation *operation, AmaObjectWork *work) {
    AmaObjectType *type = work->type;
    size_t field = work->next_field;
    AmaFieldRecipe recipe = {0};
    bool explicit_fields = work->recipe != NULL && work->recipe->fields != NULL;
    if (explicit_fields) { recipe = work->recipe->fields[field]; }
    else if (type->defaults != NULL) { recipe = type->defaults[field]; }
    if (type->resolve_field != NULL && (!explicit_fields || !recipe.supplied)) {
        if (operation->world->construction_work != NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        operation->world->construction_work = work;
        AmaStatus status = type->resolve_field(operation->world, &operation->arena,
            work->handle, field, &recipe, type->field_context);
        operation->world->construction_work = NULL;
        if (status.code != AMA_OK) { return status; }
    }
    const AmaObjectField *descriptor = &type->fields[field];
    size_t count = ama_field_list(descriptor->kind) ? recipe.count : 1;
    if (count > SIZE_MAX / sizeof(size_t)
        || (ama_field_owns(descriptor->kind) && work->cell->metadata.lifetime.child_capacity > SIZE_MAX - count)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    void *payload = ama_pool_payload(&type->pool, work->cell);
    if (ama_field_list(descriptor->kind)) {
        AmaStatus status = ama_object_list_prepare(operation->world, work, field, count);
        if (status.code != AMA_OK) { return status; }
    }
    if (ama_field_owns(descriptor->kind)) { work->cell->metadata.lifetime.child_capacity += count; }
    else if (recipe.references != NULL) {
        for (size_t index = 0; index < count; index++) {
            if (recipe.references[index].generation != 0 && recipe.references[index].type_id != descriptor->type_id) {
                return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
            }
            *ama_object_field_handle(type, payload, field, index) = recipe.references[index];
        }
    }
    work->field_recipe = recipe;
    work->field_ready = true;
    work->next_element = 0;
    return ama_status_success();
}
