#include "internal.h"

static bool ama_list_expected(AmaStatus status) {
    return status.code == AMA_BUSY || status.code == AMA_STALE_REFERENCE || status.code == AMA_EMPTY_REFERENCE;
}

static AmaStatus ama_list_preflight(AmaListMutation *mutation, const AmaListRequest *request) {
    if (!mutation->owning) { return ama_status_success(); }
    if ((request->action == AMA_LIST_APPEND || request->action == AMA_LIST_INSERT) && request->source.generation != 0) {
        return ama_object_preflight(&mutation->objects, request->source, true, false);
    }
    size_t count = request->action == AMA_LIST_CLEAR ? mutation->list->count : request->action == AMA_LIST_REMOVE ? 1 : 0;
    size_t start = request->action == AMA_LIST_REMOVE ? request->position : 0;
    for (size_t index = 0; index < count; index++) {
        AmaStatus status = ama_object_preflight(&mutation->objects, mutation->list->items[start + index], false, false);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_success();
}

static AmaStatus ama_list_mutate(AmaListMutation *mutation, const AmaListRequest *request) {
    AmaHandleList *list = mutation->list;
    switch (request->action) {
        case AMA_LIST_INSPECT: return ama_status_success();
        case AMA_LIST_APPEND: return ama_list_insert(mutation, list->count, request->source);
        case AMA_LIST_INSERT: return ama_list_insert(mutation, request->position, request->source);
        case AMA_LIST_REMOVE: return ama_list_remove(mutation, request->position, 1);
        case AMA_LIST_CLEAR: return ama_list_remove(mutation, 0, list->count);
        case AMA_LIST_SWAP: {
            AmaHandle temporary = list->items[request->position];
            list->items[request->position] = list->items[request->secondary];
            list->items[request->secondary] = temporary;
            return ama_status_success();
        }
    }
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

AmaStatus ama_list_apply(AmaWorld *world, AmaHandle owner, AmaHandleList *list, uint64_t type,
    bool owning, const AmaListRequest *request) {
    if (world->operation_active || world->failed || request->action > AMA_LIST_SWAP || request->action < AMA_LIST_INSPECT) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    bool bounds = (request->action == AMA_LIST_INSERT && request->position > list->count) ||
        ((request->action == AMA_LIST_REMOVE || request->action == AMA_LIST_SWAP) && request->position >= list->count) ||
        (request->action == AMA_LIST_SWAP && request->secondary >= list->count);
    if (bounds) { list->refused = true; list->outcome = AMA_INDEX_BOUNDS; return ama_status_success(); }
    AmaListMutation mutation = {.objects = {.world = world}, .owner = owner, .list = list, .type = type, .owning = owning};
    AmaObjectType *parent_type = NULL;
    AmaStatus status = ama_object_lookup(world, owner, false, &parent_type, &mutation.parent);
    if (status.code != AMA_OK) { return status; }
    source_arena_init(&mutation.objects.arena, world->allocator);
    world->operation_active = true;
    status = ama_list_preflight(&mutation, request);
    if (ama_list_expected(status)) { list->refused = true; list->outcome = status.code; status = ama_status_success(); }
    else if (status.code == AMA_OK) {
        status = ama_list_mutate(&mutation, request);
        if (status.code == AMA_OK) { list->refused = false; list->outcome = AMA_OK; }
    }
    ama_object_unprotect(&mutation.objects);
    source_arena_destroy(&mutation.objects.arena);
    world->operation_active = false;
    return status;
}
