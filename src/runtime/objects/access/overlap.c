/**
 * @file
 * @brief Reject overlapping ownership boundaries before copy preparation.
 *
 * [COPY-06](../../../../spec/language/values-and-references.md#copy-06)
 * Use exact handle identity and owner chains bounded by the checked occupied population; no guessed traversal timeout.
 */
#include <stdint.h>
#include "runtime/objects/internal.h"

/** Sum occupied block populations with checked size arithmetic to obtain an exact owner-chain traversal bound. */
static AmaStatus ama_world_population(const AmaWorld *world, size_t *count) {
    *count = 0;
    for (size_t type = 0; type < world->type_count; type++) {
        const AmaPool *pool = &world->types[type]->pool;
        for (size_t block = 0; block < pool->block_count; block++) {
            if (*count > SIZE_MAX - pool->blocks[block].occupied) {
                return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
            }
            *count += pool->blocks[block].occupied;
        }
    }
    return ama_status_success();
}

/** Follow validated owner links for at most the actual occupied population; exceeding that bound proves a malformed ownership cycle, not a timeout. */
static AmaStatus ama_owner_ancestor(AmaWorld *world, AmaHandle possible, AmaHandle node, size_t bound, bool *found) {
    for (size_t step = 0; step < bound; step++) {
        AmaObjectType *type;
        AmaPoolCell *cell;
        AmaStatus status = ama_object_lookup(world, node, false, &type, &cell);
        if (status.code != AMA_OK) { return status; }
        node = cell->owner;
        if (node.generation == 0) { return ama_status_success(); }
        if (ama_same_handle(node, possible)) { *found = true; return ama_status_success(); }
    }
    /* A simple owner path has at most the world's occupied-slot ama_world_population.
     * Exceeding that exact bound proves an invalid owner cycle, not a timeout. */
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

/** Check exact root equality or validated owner ancestry using the finite occupied-population bound; output is meaningful only on success. */
AmaStatus ama_object_contains(AmaWorld *world, AmaHandle root, AmaHandle node, bool *found) {
    *found = ama_same_handle(root, node);
    if (*found) { return ama_status_success(); }
    size_t bound = 0;
    AmaStatus status = ama_world_population(world, &bound);
    return status.code == AMA_OK ? ama_owner_ancestor(world, root, node, bound, found) : status;
}

/** Validate destination availability and both identities, then exact runtime self-copy and overlap. Distinct-source availability is checked by whole-tree preflight before cloning. Ordinary exact self is effect-free; continuation self and ancestor/descendant overlap fail. Static clone capability remains mandatory in the compiler. */
AmaStatus ama_object_copy_checks(AmaWorld *world, AmaHandle destination, AmaHandle source,
    bool continuation, bool *no_op) {
    *no_op = false;
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaStatus status = ama_object_lookup(world, destination, false, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_instance_access(&cell->metadata, AMA_ACCESS_READ);
    if (status.code != AMA_OK) { return status; }
    status = ama_object_lookup(world, source, false, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    if (destination.type_id != source.type_id) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (ama_same_handle(destination, source)) {
        *no_op = !continuation;
        return continuation ? ama_status_make(AMA_INVALID_STATE, NULL, 0) : ama_status_success();
    }
    size_t bound;
    status = ama_world_population(world, &bound);
    if (status.code != AMA_OK) { return status; }
    bool overlap = false;
    status = ama_owner_ancestor(world, source, destination, bound, &overlap);
    if (status.code == AMA_OK && !overlap) { status = ama_owner_ancestor(world, destination, source, bound, &overlap); }
    return status.code == AMA_OK && overlap ? ama_status_make(AMA_INVALID_STATE, NULL, 0) : status;
}
