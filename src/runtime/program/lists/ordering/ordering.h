/** @file
 * @brief Execute fixed swap and length signatures under the parent's exact grant.
 * [LIST-08](../../../../../spec/language/lists.md#list-08) preserves membership
 * identities during swap; [LIST-04](../../../../../spec/language/lists.md#list-04)
 * constructs an ordinary owned integer result without decimal conversion.
 */
#ifndef AMA_PROGRAM_LIST_ORDERING_H
#define AMA_PROGRAM_LIST_ORDERING_H
#include "runtime/program/lists/inputs/inputs.h"
/** Convert both prepared positions before publishing a single identity swap.
 * Bounds/conversion refusals preserve the entire list and create no result.
 */
AmaStatus ama_program_list_swap(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
/** Construct one fresh integer result through the fixed typed size bridge.
 * Native failures remain technical; successful ownership belongs to the frame.
 */
AmaStatus ama_program_list_length(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
#endif
