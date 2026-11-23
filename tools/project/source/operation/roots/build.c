/** @file
 * @brief Compose exact root-choice accounting with original analysis coverage before plan certification.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires the complete original domain before creating edits, independently of F.
 */
#include "internal.h"
#include <stdint.h>
SourceOperationRootsStatus source_operation_roots_build(SourceOperationRoots *owner, const SourceOperationRootsInputs *inputs) {
    if (!owner || owner->attempted) { return SOURCE_OPERATION_ROOTS_INVALID; }
    owner->attempted = true; owner->status = SOURCE_OPERATION_ROOTS_INVALID;
    if (!inputs || !inputs->configured || !inputs->configured->valid || !inputs->plan || !inputs->plan->prepared
        || inputs->original_count != inputs->configured->root_count || (inputs->original_count && !inputs->originals)
        || (inputs->addition_count && !inputs->additions)
        || inputs->addition_count > SIZE_MAX / sizeof(SourceOperationRootAddition)) { return owner->status; }
    owner->inputs = *inputs;
    owner->status = source_operation_roots_settings(owner);
    if (owner->status == SOURCE_OPERATION_ROOTS_OK) { owner->status = source_operation_roots_coverage(owner); }
    if (owner->status == SOURCE_OPERATION_ROOTS_OK) { owner->status = source_operation_roots_originals(owner); }
    owner->valid = owner->status == SOURCE_OPERATION_ROOTS_OK;
    return owner->status;
}
