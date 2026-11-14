/** @file
 * @brief Publish immutable compiler-control callbacks for each context-local type.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) separates
 * preflight, copied namespace state and reference fixups. These synchronous
 * internal phases are not native ABI slots and never execute Ama bodies.
 */
#include "internal.h"
const AmaObjectControlHooks ama_program_foreach_controls = {
    ama_foreach_control_validate, ama_foreach_control_copy, ama_foreach_control_references
};
