/** @file
 * @brief Stage fixed typed method parameters without a list action selector.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) fixes scalar input
 * arity/types; [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * separates textual source evaluation from declaration-order construction.
 */
#ifndef AMA_PROGRAM_LIST_INPUTS_H
#define AMA_PROGRAM_LIST_INPUTS_H
#include "runtime/program/lists/service.h"
#include "runtime/program/calls/call.h"

/** Resolved binding sources and exact operation sites; fields are input ordinals. */
typedef struct {
    const AmaProgramInput *inputs;
    size_t count;
    uint64_t integer_type;
    uint64_t operation;
    const uint64_t *installations;
} AmaProgramListInputs;

/** Evaluate supplied sources once, then construct owned integer/reference inputs
 * using the method's fixed field forms. At most two scalar inputs exist. Partial
 * owned credits remain in the method frame for exact cleanup on every failure.
 */
AmaStatus ama_program_list_prepare_inputs(AmaProgramListMethod *, const AmaProgramListInputs *,
    const AmaFieldKind *forms, size_t count);
/** Convert one owned integer parameter once through its explicit native protocol. */
AmaStatus ama_program_list_index(AmaProgramListMethod *, uint64_t integer_type,
    size_t input, size_t *position, bool *refused);
/** Record only a previously classified expected refusal; technical errors do not
 * gain admission provenance from their numeric status.
 */
AmaStatus ama_program_list_record_refusal(AmaProgramListMethod *, AmaStatus, bool *refused);
#endif
