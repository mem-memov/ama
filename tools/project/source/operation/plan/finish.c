/** @file
 * @brief Admit configuration expectations and mutation trees only after the single projection exists.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete read-only preparation before final compiler proof and publication.
 */
#include "internal.h"
SourceOperationPlanStatus source_operation_plan_finish(SourceOperationPlan *plan,
    const SourceOperationConfigurationExpectation *configurations, size_t count) {
    if (!plan || !plan->projected || plan->finished || plan->status != SOURCE_OPERATION_PLAN_OK) { return SOURCE_OPERATION_PLAN_INVALID; }
    plan->finished = true;
    if (count && !configurations) { return plan->status = SOURCE_OPERATION_PLAN_INVALID; }
    plan->inputs.configurations = configurations; plan->inputs.configuration_count = count;
    const SourceOperationPlanInputs *inputs = &plan->inputs;
    plan->status = source_operation_plan_configurations(plan);
    if (plan->status != SOURCE_OPERATION_PLAN_OK) { return plan->status; }
    plan->mutation_status = source_mutation_forest(&plan->arena, &inputs->observed->image, plan->image, &plan->mutations, &plan->mutation_issue);
    if (plan->mutation_status != SOURCE_FOREST_OK) { plan->failed_path = plan->mutation_issue.path; return plan->status = SOURCE_OPERATION_PLAN_MUTATIONS; }
    plan->prepared = true; return plan->status = SOURCE_OPERATION_PLAN_OK;
}
