/**
 * @file
 * @brief Stage list handles and independently constructed owned roots.
 *
 * [COPY-02](../../../../../spec/language/values-and-references.md#copy-02)
 * keeps destination storage unchanged until every prepared element succeeds.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Read the already validated source shape in its shared or mutable storage domain. */
static size_t ama_transfer_list_count(const AmaProgramValue *value) {
    return value->shared_list != NULL ? value->shared_list->count : value->list->count;
}

AmaStatus ama_transfer_list_prepare(AmaProgram *program, AmaProgramValue *destination,
    const AmaProgramValue *source, const AmaFieldRecipe *initializers, AmaObjectOperation *operation, AmaHandleList *prepared) {
    size_t count = source != NULL ? ama_transfer_list_count(source) : initializers == NULL ? 0 : initializers->count;
    if (count > SIZE_MAX / sizeof(size_t)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    const AmaHandleList *old = destination->list;
    if (old->incarnation == old->counter_limit) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    AmaStatus prepared_status = ama_handle_list_prepare(prepared, program->world->allocator,
        old->owner, old->field, old->incarnation + 1, old->counter_limit, 0);
    if (prepared_status.code != AMA_OK) { return prepared_status; }
    for (size_t index = 0; index < count; index++) {
        AmaMembership identity;
        AmaStatus inserted = ama_membership_insert(prepared, prepared->count, (AmaHandle){0}, &identity);
        if (inserted.code != AMA_OK) { return inserted; }
        AmaHandle handle = {0};
        if (destination->form == AMA_FIELD_REFERENCE_LIST) {
            handle = source != NULL ? (source->shared_list != NULL ? (AmaHandle){0} : *ama_handle_list_element(source->list, index))
                : initializers != NULL && initializers->references != NULL ? initializers->references[index] : (AmaHandle){0};
        } else {
            AmaRecipe recipe = {0};
            AmaStatus status = ama_status_success();
            if (source != NULL) {
                AmaProgramValue element = {.type_id = source->type_id, .form = AMA_FIELD_OWNED};
                if (source->shared_list != NULL) { element.constant = &source->shared_list->items[index]; }
                else { element.handle = *ama_handle_list_element(source->list, index); }
                status = ama_program_value_recipe(program, &operation->arena, &element, source->continuation, &recipe);
            } else if (initializers != NULL && initializers->children != NULL) { recipe = initializers->children[index]; }
            if (status.code == AMA_OK) { status = ama_object_build(operation, destination->type_id, &recipe, &handle); }
            if (status.code != AMA_OK) {
                (void)ama_membership_remove(prepared, prepared->count - 1, 1);
                return status;
            }
        }
        *ama_handle_list_element(prepared, prepared->count - 1) = handle;
    }
    return ama_status_success();
}
