/** @file
 * @brief Admit independent final compiler evidence for the exact fully prepared operation image.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates read-only plan construction, semantic proof and later exclusive publication.
 */
#include "plan.h"
SourceCopyProofStatus source_operation_plan_prove(SourceOperationPlan *plan, const SourceCompilerClient *client, SourceCopyProofIssue *issue) {
    if (!plan || !plan->prepared) {
        if (issue) { *issue = (SourceCopyProofIssue){0}; } return SOURCE_COPY_PROOF_INVALID;
    }
    const SourceOperationPlanInputs *inputs = &plan->inputs;
    return source_copy_prove(&plan->arena, inputs->original, inputs->observed,
        inputs->mains, inputs->files, inputs->texts, inputs->contexts, &plan->capture, client,
        inputs->removal ? inputs->removal->identities : NULL,
        inputs->removal ? inputs->removal->identity_count : 0, issue);
}
