/** @file
 * @brief Prepare typed append/insert inputs and publish one exact new membership.
 * [LIST-09](../../../../../spec/language/lists.md#list-09) distinguishes omission,
 * owning clone and reference copy; [LIST-10](../../../../../spec/language/lists.md#list-10)
 * separates checked preflight refusal from native/resource failure.
 */
#ifndef AMA_PROGRAM_LIST_INSERTION_H
#define AMA_PROGRAM_LIST_INSERTION_H
#include "runtime/program/lists/service.h"
#include "runtime/program/calls/call.h"

/** Resolved textual bindings plus exact standard integer identity and operation
 * sites. Input field numbers are interface ordinals, never action constants.
 */
typedef struct {
    const AmaProgramInput *inputs;
    size_t count;
    uint64_t integer_type;
    uint64_t operation;
    const uint64_t *installations;
} AmaProgramListInputs;

/** Append with one optional element-reference input under an active parent grant.
 * The method retains prepared inputs/result until explicit method_end, including
 * after expected refusal. Only an admitted successful mutation publishes a result.
 */
AmaStatus ama_program_list_append(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
/** Insert with integer position followed by optional element reference. Conversion
 * uses the declared read-only protocol once; no payload layout is assumed.
 */
AmaStatus ama_program_list_insert(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
#endif
