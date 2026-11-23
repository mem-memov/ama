/** @file
 * @brief Prove complete final package-scope accounting and exact independently reparsed root sets.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids permission broadening through wildcards even in an otherwise compilable image.
 */
#include <string.h>
#include "internal.h"
/** Validate actual owned scope before using its identity as an expectation key. */
static bool owned(const SourceCopyConfigurationExpectation *expected) {
    if (!expected->environment || !expected->environment->valid) { return false; }
    for (const SourcePackageScope *scope = expected->environment->first; scope; scope = scope->next) {
        if (scope == expected->scope) { return true; }
    }
    return false;
}
/** Whole selected versions have unique complete nominal names and physical roots. */
static bool same_scope(const SourcePackageScope *left, const SourcePackageScope *right) {
    return !strcmp(left->version->logical_path, right->version->logical_path)
        && !strcmp(left->version->physical_path, right->version->physical_path);
}
SourceCopyPlanStatus source_copy_plan_configurations(SourceCopyPlan *plan) {
    const SourceCopyPlanInputs *inputs = &plan->inputs;
    for (size_t i = 0; i < inputs->configuration_count; i++) {
        const SourceCopyConfigurationExpectation *expected = &inputs->configurations[i];
        if (!owned(expected)) { return SOURCE_COPY_PLAN_CONFIGURATION; }
        for (size_t j = 0; j < i; j++) {
            if (same_scope(expected->scope, inputs->configurations[j].scope)) { return SOURCE_COPY_PLAN_CONFIGURATION; }
        }
        const SourcePackageScope *actual = plan->environment.first;
        while (actual && !same_scope(actual, expected->scope)) { actual = actual->next; }
        plan->failed_scope = expected->scope;
        if (!actual) { return SOURCE_COPY_PLAN_CONFIGURATION; }
        plan->configuration_status = source_copy_configuration_check(expected->environment, expected->scope, &expected->roots,
            &plan->environment, actual, &plan->configuration_issue);
        if (plan->configuration_status != SOURCE_CONFIGURATION_CHECK_OK) { return SOURCE_COPY_PLAN_CONFIGURATION; }
    }
    for (const SourcePackageScope *actual = plan->environment.first; actual; actual = actual->next) {
        size_t i = 0;
        while (i < inputs->configuration_count && !same_scope(actual, inputs->configurations[i].scope)) { i++; }
        if (i == inputs->configuration_count) { plan->failed_scope = actual; return SOURCE_COPY_PLAN_CONFIGURATION; }
    }
    plan->failed_scope = NULL; return SOURCE_COPY_PLAN_OK;
}
