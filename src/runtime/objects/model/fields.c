/**
 * @file
 * @brief Interpret validated field descriptors and initialized owned-child positions.
 *
 * [LIST-13](../../../../spec/language/lists.md#list-13) reserves exact replacement incarnations.
 * [LIFE-02](../../../../spec/implementation/native-interface.md#life-02)
 * Flatten only declared owned storage; reference targets are never child ownership edges.
 */
#include <stdint.h>
#include <string.h>
#include "runtime/objects/internal.h"

/** Read the current declared shape without resolving any target. */
size_t ama_object_field_count(const AmaObjectType *type, void *payload, size_t field) {
    void *address = (unsigned char *)payload + type->fields[field].offset;
    return ama_field_list(type->fields[field].kind) ? ((AmaHandleList *)address)->count : 1;
}

/** Select a scalar or one protected current record target; indices are prevalidated. */
AmaHandle *ama_object_field_handle(const AmaObjectType *type, void *payload, size_t field, size_t index) {
    void *address = (unsigned char *)payload + type->fields[field].offset;
    return ama_field_list(type->fields[field].kind) ? ama_handle_list_element(address, index) : address;
}

/** New private roots get a fresh namespace. A stable destination replacement must
 * reserve its old field's next incarnation while the old payload is still live.
 * [LIST-13](../../../../spec/language/lists.md#list-13) forbids reviving old cursors.
 */
AmaStatus ama_object_list_prepare(AmaWorld *world, AmaObjectWork *work, size_t field, size_t count) {
    uint64_t incarnation = 1, limit = UINT64_MAX;
    if (!ama_handle_equal(work->identity, work->handle)) {
        AmaPoolCell *old = NULL;
        AmaStatus status = ama_pool_lookup(&work->type->pool, work->identity, false, &old);
        if (status.code != AMA_OK) { return status; }
        AmaHandleList *previous = (void *)((unsigned char *)ama_pool_payload(&work->type->pool, old)
            + work->type->fields[field].offset);
        limit = previous->counter_limit;
        if (previous->incarnation == limit) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        incarnation = previous->incarnation + 1;
    }
    AmaHandleList *list = (void *)((unsigned char *)ama_pool_payload(&work->type->pool, work->cell)
        + work->type->fields[field].offset);
    return ama_handle_list_prepare(list, world->allocator, work->identity, field, incarnation, limit, count);
}

/** Validate supplied recipe field count and begin the private lifecycle ledger; construction phases populate and credit the exact owned-child prefix. */
AmaStatus ama_object_fields(AmaWorld *world, AmaObjectWork *work) {
    (void)world;
    if (work->recipe != NULL && work->recipe->fields != NULL && work->recipe->field_count != work->type->field_count) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaConstructorKind kind = work->recipe == NULL ? AMA_CONSTRUCT_DEFAULT : work->recipe->constructor;
    return ama_lifetime_begin(&work->cell->metadata.lifetime, 0, kind);
}

/** Resolve one valid flattened owned-child index to its declared handle location/type. Reference fields are skipped; the caller supplies an initialized-prefix index. */
AmaHandle *ama_object_child(AmaObjectWork *work, size_t index, uint64_t *type_id, const AmaRecipe **recipe) {
    void *payload = ama_pool_payload(&work->type->pool, work->cell);
    for (size_t field = 0; field < work->type->field_count; field++) {
        if (!ama_field_owns(work->type->fields[field].kind)) { continue; }
        size_t count = ama_object_field_count(work->type, payload, field);
        if (index < count) {
            *type_id = work->type->fields[field].type_id;
            *recipe = NULL;
            return ama_object_field_handle(work->type, payload, field, index);
        }
        index -= count;
    }
    return NULL;
}
