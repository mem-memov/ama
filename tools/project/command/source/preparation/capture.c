/** @file
 * @brief Admit one immutable parsed source request before capturing its common original domain.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps failed original preparation unaccepted and separate from final authority.
 */
#include "internal.h"
ProjectSourcePreparationStatus project_source_preparation_capture(ProjectSourcePreparation *owner,
    const ProjectSourcePreparationInputs *inputs) {
    if (!owner || owner->attempted) { return PROJECT_SOURCE_PREPARATION_INVALID; }
    owner->attempted = true;
    if (!inputs || !inputs->request || !inputs->request->valid
        || !inputs->temporary_parent || !*inputs->temporary_parent) { return owner->status = PROJECT_SOURCE_PREPARATION_INVALID; }
    owner->inputs = *inputs;
    owner->status = project_source_preparation_original(owner);
    owner->captured = owner->status == PROJECT_SOURCE_PREPARATION_OK;
    return owner->status;
}
