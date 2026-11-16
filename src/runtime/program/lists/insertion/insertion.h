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
#include "runtime/program/lists/inputs/inputs.h"

/** Append with one optional element-reference input under an active parent grant.
 * The method retains prepared inputs/result until explicit method_end, including
 * after expected refusal. Only an admitted successful mutation publishes a result.
 * Component contract: [LIST-09](../../../../../spec/language/lists.md#list-09), [LIST-10](../../../../../spec/language/lists.md#list-10). */
AmaStatus ama_program_list_append(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
/** Insert with integer position followed by optional element reference. Conversion
 * uses the declared read-only protocol once; no payload layout is assumed.
 * Component contract: [LIST-09](../../../../../spec/language/lists.md#list-09), [LIST-10](../../../../../spec/language/lists.md#list-10). */
AmaStatus ama_program_list_insert(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
#endif
