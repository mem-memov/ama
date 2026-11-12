/**
 * @file
 * @brief Check clone capability over the exact reachable declared ownership graph.
 *
 * [COPY-06](../../../../spec/language/values-and-references.md#copy-06)
 * Empty owning lists still impose element-type capability. A finite monotone reachability set replaces depth limits or heuristics.
 */
#include <stdint.h>
#include <string.h>
#include "runtime/objects/internal.h"

/** Find the exact nominal ID in the unique borrowed registry, returning type_count if absent; linear scan, no inferred identity. */
static size_t ama_world_type_index(const AmaWorld *world, uint64_t id) {
    for (size_t index = 0; index < world->type_count; index++) {
        if (world->types[index]->pool.type_id == id) { return index; }
    }
    return world->type_count;
}

/** Compute finite monotone reachability over every declared owned field, including empty-list element types. Reject any reachable native resource lacking clone; scratch remains operation-owned. */
AmaStatus ama_object_copy_capability(AmaObjectOperation *operation, uint64_t root) {
    AmaWorld *world = operation->world;
    size_t index = ama_world_type_index(world, root);
    if (index == world->type_count) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (world->type_count > SIZE_MAX / sizeof(bool)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    size_t bytes = world->type_count * sizeof(bool);
    bool *reachable = source_arena_allocate(&operation->arena, bytes);
    if (reachable == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    memset(reachable, 0, bytes);
    reachable[index] = true;
    bool changed;
    do {
        changed = false;
        for (size_t current = 0; current < world->type_count; current++) {
            if (!reachable[current]) { continue; }
            const AmaObjectType *type = world->types[current];
            if (type->hooks.initialize != NULL && type->hooks.clone == NULL) {
                return ama_status_make(AMA_INVALID_STATE, NULL, 0);
            }
            for (size_t field = 0; field < type->field_count; field++) {
                if (!ama_field_owns(type->fields[field].kind)) { continue; }
                size_t child = ama_world_type_index(world, type->fields[field].type_id);
                if (child == world->type_count) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
                if (!reachable[child]) { reachable[child] = true; changed = true; }
            }
        }
    } while (changed);
    return ama_status_success();
}
