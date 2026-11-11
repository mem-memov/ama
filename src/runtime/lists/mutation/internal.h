#ifndef AMA_RUNTIME_LIST_MUTATION_INTERNAL_H
#define AMA_RUNTIME_LIST_MUTATION_INTERNAL_H
#include "runtime/lists/list.h"
#include "runtime/objects/internal.h"
typedef struct {
    AmaObjectOperation objects;
    AmaHandle owner;
    AmaHandleList *list;
    uint64_t type;
    bool owning;
    AmaPoolCell *parent;
} AmaListMutation;
AmaStatus ama_list_insert(AmaListMutation *, size_t, AmaHandle);
AmaStatus ama_list_remove(AmaListMutation *, size_t, size_t);
#endif
