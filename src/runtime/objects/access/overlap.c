#include <stdint.h>
#include "runtime/objects/internal.h"

static AmaStatus population(const AmaWorld *world, size_t *count) {
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

static AmaStatus ancestor(AmaWorld *world, AmaHandle possible, AmaHandle node, size_t bound, bool *found) {
    for (size_t step = 0; step < bound; step++) {
        AmaObjectType *type;
        AmaPoolCell *cell;
        AmaStatus status = ama_object_lookup(world, node, false, &type, &cell);
        if (status.code != AMA_OK) { return status; }
        node = cell->owner;
        if (node.generation == 0) { return ama_status_success(); }
        if (ama_same_handle(node, possible)) { *found = true; return ama_status_success(); }
    }
    /* A simple owner path has at most the world's occupied-slot population.
     * Exceeding that exact bound proves an invalid owner cycle, not a timeout. */
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

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
    status = population(world, &bound);
    if (status.code != AMA_OK) { return status; }
    bool overlap = false;
    status = ancestor(world, source, destination, bound, &overlap);
    if (status.code == AMA_OK && !overlap) { status = ancestor(world, destination, source, bound, &overlap); }
    return status.code == AMA_OK && overlap ? ama_status_make(AMA_INVALID_STATE, NULL, 0) : status;
}
