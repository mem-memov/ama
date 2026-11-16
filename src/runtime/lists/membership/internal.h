/** @file
 * @brief Share trusted record allocation and exact order invariants.
 * [LIST-03](../../../../spec/language/lists.md#list-03) fixes a bijection between
 * order positions and live records; allocation prepares storage before mutation.
 */
#ifndef AMA_RUNTIME_MEMBERSHIP_INTERNAL_H
#define AMA_RUNTIME_MEMBERSHIP_INTERNAL_H
#include "membership.h"

/** Fixed block size keeps every borrowed record stable across directory growth. Component contract: [LIST-03](../../../../spec/language/lists.md#list-03). */
#define AMA_MEMBERSHIP_BLOCK_SIZE 32u
/** Resolve a caller-validated record index without a target search. Component contract: [LIST-03](../../../../spec/language/lists.md#list-03). */
static inline AmaMembershipRecord *ama_membership_record(const AmaMembershipList *list, size_t index) {
    return &list->blocks[index / AMA_MEMBERSHIP_BLOCK_SIZE][index % AMA_MEMBERSHIP_BLOCK_SIZE];
}
/** Reconstruct only from the current live record while authority is held. Component contract: [LIST-03](../../../../spec/language/lists.md#list-03). */
static inline AmaMembership ama_membership_identity(const AmaMembershipList *list, size_t index) {
    return (AmaMembership){list->owner, list->field, list->incarnation, index,
        ama_membership_record(list, index)->generation};
}
/** Prepare capacity and a free record; no live identity/order is changed. Component contract: [LIST-03](../../../../spec/language/lists.md#list-03). */
AmaStatus ama_membership_prepare(AmaMembershipList *, size_t required);
/** Consume one prepared free record; commit path cannot allocate or fail. Component contract: [LIST-03](../../../../spec/language/lists.md#list-03). */
size_t ama_membership_take(AmaMembershipList *, size_t position, AmaHandle);
/** Invalidate before reuse; exhausted generations are never put on the free list. Component contract: [LIST-03](../../../../spec/language/lists.md#list-03). */
void ama_membership_retire(AmaMembershipList *, size_t record);
#endif
