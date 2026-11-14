/** @file
 * @brief Remove checked memberships under typed service authority.
 * [LIST-06](../../../../../spec/language/lists.md#list-06) preflights the complete
 * owning deletion forest; reference removal retires occurrences without targets.
 */
#ifndef AMA_PROGRAM_LIST_DELETION_H
#define AMA_PROGRAM_LIST_DELETION_H
#include "runtime/program/lists/inputs/inputs.h"

/** Remove the converted current position; no declared method result is created. */
AmaStatus ama_program_list_remove(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
/** Preflight every owning root before clearing any occurrence. Empty succeeds
 * without a target effect or an irrelevant integer input/default/conversion.
 */
AmaStatus ama_program_list_clear(AmaProgramListService *, AmaProgramListMethod *,
    const AmaProgramListInputs *, bool *refused);
#endif
