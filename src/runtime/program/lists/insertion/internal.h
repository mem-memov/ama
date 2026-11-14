/** @file
 * @brief Share the bounded typed input view for contiguous insertion preparation.
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08) resolves
 * sources in textual order before declaration-order construction.
 */
#ifndef AMA_PROGRAM_LIST_INSERTION_INTERNAL_H
#define AMA_PROGRAM_LIST_INSERTION_INTERNAL_H
#include "insertion.h"

/** Borrowed source views live only during the contiguous C helper, before any
 * Ama label transfer; owned input roots belong to the stable method frame.
 */
typedef struct {
    const AmaProgramInput *supplied[2];
    AmaProgramValue values[2];
    size_t arity;
    size_t source_position;
    bool positioned;
} AmaListInsertionInputs;

/** Select once in source order, then construct only the declared owned input. */
AmaStatus ama_list_insertion_inputs(AmaProgramListMethod *, const AmaProgramListInputs *, AmaListInsertionInputs *);
/** Convert/check position, stage and publish one element; preserve explicit
 * provenance and record only expected preflight refusal on the initiating caller.
 */
AmaStatus ama_list_insertion_commit(AmaProgramListMethod *, const AmaProgramListInputs *,
    const AmaListInsertionInputs *, bool *refused);
#endif
