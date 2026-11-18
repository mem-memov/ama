/** @file
 * @brief Intern expression uses with exact lexical creation and definition contexts.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves source operators.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** A definition RHS belongs to its own exact environment; foreign reads first
 * enter the target definition rather than importing its expression into this one.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves lexical scope.
 */
static bool definition_path(const StaticResultPlan *plan, const StaticResultContext *context,
    const StaticDefinitionPath *definition) {
    if (definition == NULL) { return true; }
    for (const StaticDefinitionPath *path = plan->guards->first_definition_path; path; path = path->next) {
        if (path == definition) {
            return path->node->owner->environment == context->creation->owner->environment;
        }
    }
    return false;
}

/** Repeated uses share the AST/environment variable but retain distinct actual
 * definition ancestry. Application preparation is a separate later operation.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves work identity.
 */
bool static_result_use(StaticResultPlan *plan, const StaticResultContext *context,
    const AstTypeExpression *expression, const StaticDefinitionPath *definition, StaticResultUse **output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !static_result_has_context(plan, context) || !definition_path(plan, context, definition)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid scoped expression-use context");
    }
    const StaticResultSource *source;
    if (!static_result_source(plan, context->creation->owner->environment, expression, &source)) { return false; }
    for (StaticResultUse *use = plan->first_use; use; use = use->next) {
        if (use->source == source && use->context == context && use->definition_path == definition) { *output = use; return true; }
    }
    if (plan->use_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, expression->span,
            "Static expression-use count exceeds representable storage");
    }
    StaticResultUse *use = static_result_allocate(plan, sizeof(*use));
    if (use == NULL) { return false; }
    *use = (StaticResultUse){.source = source, .context = context, .definition_path = definition,
        .outcome = STATIC_RESULT_PENDING, .next = plan->first_use};
    plan->first_use = use;
    plan->use_count++;
    *output = use;
    return true;
}
