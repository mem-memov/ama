/**
 * @file
 * @brief Declare private pool address lookup and growth operations.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Borrow an initialized pool with consistent directory metadata; no lifecycle hooks or body dispatch belong here.
 */
#ifndef AMA_RUNTIME_POOL_INTERNAL_H
#define AMA_RUNTIME_POOL_INTERNAL_H
#include "pool.h"
/** Borrow the cell at a bounded historical slot, or null for missing/reclaimed storage; no publication or generation validation is implied. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
AmaPoolCell *ama_pool_cell(const AmaPool *, size_t slot);
/** Append one checked aligned block with fresh historical slot IDs and generation one. Failure leaves existing cells valid; reclaimed IDs are never reissued. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
AmaStatus ama_pool_grow(AmaPool *);
#endif
