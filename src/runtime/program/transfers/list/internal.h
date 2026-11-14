/**
 * @file
 * @brief Declare list transfer phases and explicit staging ownership handoffs.
 *
 * [COPY-02](../../../../../spec/language/values-and-references.md#copy-02)
 * preserves the old value until staging succeeds; destructive failure is terminal.
 */
#ifndef AMA_LIST_TRANSFER_INTERNAL_H
#define AMA_LIST_TRANSFER_INTERNAL_H
#include "runtime/program/transfers/internal.h"

/** Validate shapes/aliases and protect needed owned trees. Caller owns any partial
 * protections. no_op is true only for ordinary identical-list assignment.
 */
AmaStatus ama_transfer_list_validate(AmaProgramValue *, const AmaProgramValue *, AmaObjectOperation *, bool *no_op);
/** Stage from a checked source/default. prepared starts empty, owns its allocated
 * buffer and successful roots even on failure; the current failed root unwinds
 * inside object construction. It must later be committed or discarded.
 */
AmaStatus ama_transfer_list_prepare(AmaProgram *, AmaProgramValue *, const AmaProgramValue *,
    const AmaFieldRecipe *, AmaObjectOperation *, AmaHandleList *prepared);
/** Join completed owned roots into one map and fix references. All operation work
 * belongs to this transfer; copied_source distinguishes copy from independent defaults.
 */
AmaStatus ama_transfer_list_fixup(AmaObjectOperation *, bool copied_source);
/** Check parent/counts, destroy old storage and publish staging. Success consumes
 * prepared; failure leaves its ownership with caller for discard. Earlier old-value
 * destruction cannot be rolled back; primary cleanup errors remain unchanged.
 */
AmaStatus ama_transfer_list_commit(AmaProgram *, AmaProgramValue *, AmaObjectOperation *, AmaHandleList *prepared);
/** Free a completed prefix/buffer and zero its owner; preserve primary errors and
 * continue after destructor failure. Nonowning handles never destroy referents.
 */
AmaStatus ama_transfer_list_discard(AmaProgram *, AmaHandleList *, bool owning, AmaStatus primary);
#endif
