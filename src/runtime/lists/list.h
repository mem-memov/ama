#ifndef AMA_RUNTIME_LIST_H
#define AMA_RUNTIME_LIST_H
#include "runtime/objects/objects.h"
typedef enum { AMA_LIST_INSPECT, AMA_LIST_APPEND, AMA_LIST_INSERT, AMA_LIST_REMOVE, AMA_LIST_CLEAR, AMA_LIST_SWAP } AmaListAction;
typedef struct { AmaListAction action; size_t position, secondary; AmaHandle source; } AmaListRequest;
/* Technical failures return status. Expected refusals update list results only. */
AmaStatus ama_list_apply(AmaWorld *, AmaHandle owner, AmaHandleList *, uint64_t element_type,
    bool owning, const AmaListRequest *);
#endif
