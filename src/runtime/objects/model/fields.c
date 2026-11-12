/**
 * @file
 * @brief Interpret validated field descriptors and initialized owned-child positions.
 *
 * [LIFE-02](../../../../spec/implementation/native-interface.md#life-02)
 * Flatten only declared owned storage; reference targets are never child ownership edges.
 */
#include <stdint.h>
#include <string.h>
#include "runtime/objects/internal.h"

/** Borrow one validated scalar field location or list array and count. The descriptor index/layout must be valid and returned storage expires with its owner or array mutation. */
AmaHandle *ama_object_handles(const AmaObjectType *type, void *payload, size_t field, size_t *count) {
    void *address = (unsigned char *)payload + type->fields[field].offset;
    if (ama_field_list(type->fields[field].kind)) {
        AmaHandleList *list = address;
        *count = list->count;
        return list->items;
    }
    *count = 1;
    return address;
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
        size_t count;
        AmaHandle *handles = ama_object_handles(work->type, payload, field, &count);
        if (index < count) {
            *type_id = work->type->fields[field].type_id;
            *recipe = NULL;
            return &handles[index];
        }
        index -= count;
    }
    return NULL;
}
