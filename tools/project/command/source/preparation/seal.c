/** @file
 * @brief Check the actual complete-owner final view and seal its one immutable certificate.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids authorizing a different final capture or treating serialization as proof.
 */
#include "internal.h"
ProjectSourcePreparationStatus project_source_preparation_seal(ProjectSourcePreparation *owner) {
    if (source_workspace_snapshot(&owner->workspace, SOURCE_WORKSPACE_FINAL, &owner->complete.plan.capture.view) != SOURCE_WORKSPACE_OK) {
        owner->failed_path = owner->workspace.issue.path; return PROJECT_SOURCE_PREPARATION_WORKSPACE;
    }
    if (source_compiler_client_run(&owner->final, owner->inputs.request->compiler,
        owner->workspace.files[SOURCE_WORKSPACE_FINAL].entry.path, &owner->complete.plan.capture.view, NULL) != SOURCE_COMPILER_OK) {
        return PROJECT_SOURCE_PREPARATION_COMPILER;
    }
    if (source_operation_complete_seal(&owner->complete, &owner->final) != SOURCE_OPERATION_COMPLETE_OK) { return PROJECT_SOURCE_PREPARATION_SEAL; }
    return PROJECT_SOURCE_PREPARATION_OK;
}
