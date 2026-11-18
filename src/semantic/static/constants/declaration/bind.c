/** @file
 * @brief Bind exact shared declaration Type obligations without interpreting initializer values.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) substitutes interface parameters into shared types.
 * [CONST-01](../../../../../spec/language/declarations.md#const-01) forbids reference/reference-list constants.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial construction.
 */
#include <stdint.h>
#include "../internal.h"

/** Original top-level shared membership precedes dereferencing a caller's
 * declaration identity; ordinary fields/results cannot become constants.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) scopes shared declarations.
 */
static bool shared(const StaticResultContext *context, const AstNode *candidate) {
    for (const AstNode *node = context->creation->owner->environment->definition->definition->syntax.constants.first;
        node; node = node->next) {
        if (node == candidate) { return node->kind == AST_DECLARATION; }
    }
    return false;
}

/** A stored value or list element requires Type independently of its eventual
 * initializer bytes. The obligation remains symbolic until permitted evidence
 * determines it; binding does not expand the expression or create an instance.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) retains unknown dependent Types.
 */
bool static_constant_declaration(StaticConstantPlan *plan, const StaticResultContext *context,
    const AstNode *syntax, const StaticConstantDeclaration **output) {
    if (!static_constant_dependencies(plan)) { return false; }
    StaticResultPlan *results = plan->publications->results;
    if (output == NULL || !static_result_has_context(results, context) || !shared(context, syntax)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid shared declaration identity or scope");
    }
    if (syntax->as.declaration.name->reference) {
        return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, syntax->as.declaration.name->span,
            "Shared declarations cannot use reference or reference-list storage");
    }
    for (const StaticConstantDeclaration *record = plan->first_declaration; record; record = record->next) {
        if (record->context == context && record->syntax == syntax) { *output = record; return true; }
    }
    if (plan->declaration_count == SIZE_MAX) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, syntax->span,
            "Shared declaration metadata count exceeds representable storage");
    }
    StaticResultUse *source;
    if (!static_result_use(results, context, syntax->as.declaration.type, NULL, &source)) {
        (void)static_constant_dependencies(plan); return false;
    }
    StaticConstraintSolver *solver = results->guards->applications->solver;
    if (!static_constraint_describe(solver, source->source->value, STATIC_CATEGORY_TYPE, NULL, syntax->as.declaration.type->span) ||
        !static_constraint_require(solver, source->source->value, syntax->span)) {
        (void)static_constant_dependencies(plan); return false;
    }
    StaticConstantDeclaration *record = static_constant_allocate(plan, sizeof(*record));
    if (record == NULL) { return false; }
    *record = (StaticConstantDeclaration){.context = context, .syntax = syntax, .source = source,
        .type = source->source->value, .list = syntax->as.declaration.name->whole_list, .next = plan->first_declaration};
    plan->first_declaration = record;
    plan->declaration_count++;
    *output = record;
    return true;
}
