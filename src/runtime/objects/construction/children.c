/**
 * @file
 * @brief Prepare field progress and reserve the next owned child without recursion.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires explicit lifecycle ordering.
 * Install a reserved child handle even when its field preparation fails, so ancestry cleanup can find it.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include "internal.h"

AmaStatus ama_construction_prepare_next(AmaObjectOperation *operation, AmaObjectWork *work, bool *skipped) {
    *skipped = false;
    bool clone_source = work->source.generation != 0;
    if (!clone_source && work->next_field < work->type->field_count && !work->field_ready) {
        AmaStatus status = ama_construction_prepare_field(operation, work);
        if (status.code != AMA_OK) { return status; }
    }
    if (!clone_source && work->next_field < work->type->field_count) {
        const AmaObjectField *field = &work->type->fields[work->next_field];
        size_t count;
        (void)ama_object_handles(work->type, ama_pool_payload(&work->type->pool, work->cell), work->next_field, &count);
        if (!ama_field_owns(field->kind) || work->next_element == count) {
            work->next_field++;
            work->field_ready = false;
            *skipped = true;
        }
    }
    return ama_status_success();
}

AmaStatus ama_construction_descend(AmaObjectOperation *operation, AmaObjectWork **cursor) {
    AmaObjectWork *work = *cursor;
    uint64_t child_type;
    const AmaRecipe *child_recipe;
    AmaHandle *location;
    if (work->source.generation != 0) {
        location = ama_object_child(work, work->next_child, &child_type, &child_recipe);
    } else {
        size_t count;
        AmaHandle *handles = ama_object_handles(work->type, ama_pool_payload(&work->type->pool, work->cell), work->next_field, &count);
        location = &handles[work->next_element];
        child_type = work->type->fields[work->next_field].type_id;
        child_recipe = work->field_recipe.children == NULL ? NULL : &work->field_recipe.children[work->next_element];
    }
    AmaHandle child_source = {0};
    if (work->source.generation != 0) {
        AmaPoolCell *original = NULL;
        /* The complete source tree is already protected by reservation preflight. */
        (void)ama_pool_lookup(&work->type->pool, work->source, true, &original);
        AmaObjectWork source_work = {.type = work->type, .cell = original};
        child_source = *ama_object_child(&source_work, work->next_child, &child_type, &child_recipe);
        child_recipe = NULL;
    }
    AmaObjectWork *child_work = NULL;
    AmaStatus status = ama_construction_reserve(operation, child_type, child_recipe, child_source, work, &child_work);
    if (child_work != NULL) {
        *location = child_work->handle;
        *cursor = child_work;
    }
    return status;
}
