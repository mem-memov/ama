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
    if (count > SIZE_MAX / sizeof(AmaHandle)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    if (count != 0) {
        prepared->items = program->world->allocator.allocate(program->world->allocator.context, count * sizeof(AmaHandle));
        if (prepared->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
        memset(prepared->items, 0, count * sizeof(AmaHandle));
    }
    prepared->capacity = count;
    for (size_t index = 0; index < count; index++) {
        AmaHandle handle = {0};
        if (destination->form == AMA_FIELD_REFERENCE_LIST) {
            handle = source != NULL ? (source->shared_list != NULL ? (AmaHandle){0} : source->list->items[index])
                : initializers != NULL && initializers->references != NULL ? initializers->references[index] : (AmaHandle){0};
        } else {
            AmaRecipe recipe = {0};
            AmaStatus status = ama_status_success();
            if (source != NULL) {
                AmaProgramValue element = {.type_id = source->type_id, .form = AMA_FIELD_OWNED};
                if (source->shared_list != NULL) { element.constant = &source->shared_list->items[index]; }
                else { element.handle = source->list->items[index]; }
                status = ama_program_value_recipe(program, &operation->arena, &element, source->continuation, &recipe);
            } else if (initializers != NULL && initializers->children != NULL) { recipe = initializers->children[index]; }
            if (status.code == AMA_OK) { status = ama_object_build(operation, destination->type_id, &recipe, &handle); }
            if (status.code != AMA_OK) { return status; }
        }
        prepared->items[prepared->count++] = handle;
    }
    return ama_status_success();
}
