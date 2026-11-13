/**
 * @file
 * @brief Reject typed source kinds whose executable identity support has not reached the runtime boundary.
 * [REF-01](../../../spec/language/values-and-references.md#ref-01) gives self
 * valid static meaning now; [LIFE-02](../../../spec/implementation/native-interface.md#life-02)
 * and [COPY-03](../../../spec/language/values-and-references.md#copy-03) require
 * final construction/clone identities before that source may execute.
 */
#include "planning/internal.h"
/** An explicit unsupported diagnostic preserves the typed source site and prevents a fake access descriptor from reaching generation. */
static bool source(ProgramPlan *plan, const SemanticSource *value) {
    if (value == NULL || value->kind != AST_SELF_REFERENCE_SOURCE) { return true; }
    plan_fail(plan, value->site, "Standalone self-reference execution awaits final construction and clone identity support");
    plan->error.code = DIAGNOSTIC_UNSUPPORTED_FEATURE;
    return false;
}
bool plan_validate_sources(ProgramPlan *plan) {
    for (const SemanticType *type = plan->semantic->first; type != NULL; type = type->next) {
        for (const SemanticSymbol *field = type->first; field != NULL; field = field->next) {
            for (const SemanticInitializer *value = field->initializers; value != NULL; value = value->next) {
                if (!source(plan, value->source)) { return false; }
            }
        }
        for (const SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
            if (!source(plan, statement->source)) { return false; }
            for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
                if (!source(plan, binding->source)) { return false; }
            }
        }
    }
    return true;
}
