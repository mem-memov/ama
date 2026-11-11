#ifndef AMA_RUNTIME_POOL_INTERNAL_H
#define AMA_RUNTIME_POOL_INTERNAL_H
#include "pool.h"
AmaPoolCell *ama_pool_cell(const AmaPool *, size_t slot);
AmaStatus ama_pool_grow(AmaPool *);
#endif
