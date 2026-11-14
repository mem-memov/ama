/** @file
 * @brief Share the bounded typed input view for contiguous insertion preparation.
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08) resolves
 * sources in textual order before declaration-order construction.
 */
#ifndef AMA_PROGRAM_LIST_INSERTION_INTERNAL_H
#define AMA_PROGRAM_LIST_INSERTION_INTERNAL_H
#include "insertion.h"

/** Convert/check position, stage and publish one element; preserve explicit
 * provenance and record only expected preflight refusal on the initiating caller.
 */
AmaStatus ama_list_insertion_commit(AmaProgramListMethod *, const AmaProgramListInputs *,
    bool positioned, bool *refused);
#endif
