/** @file
 * @brief Derive contextual omission from original lexical syntax rather than lookup guesses.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) distinguishes field and partial omission.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) gives every ordinary
 * declaration the same owner-plus-field inference identity, including inputs.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) overrides every context for output phases.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) keeps namespaces independent.
 */
#include "internal.h"

/** Registered source role determines policy; namespace sources embedded in a
 * declaration remain namespaces rather than becoming that declaration's field.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) forbids namespace field inference.
 */
bool static_application_context(StaticApplicationPlan *plan, const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, StaticApplicationContext *output) {
    if (!static_application_dependencies(plan)) { return false; }
    if (output == NULL || expression == NULL || !static_term_has_environment(plan->terms, environment)) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, (SourceSpan){0},
            "Invalid static application source context", NULL);
    }
    for (const StaticOccurrence *site = environment->definition->first_occurrence; site; site = site->next) {
        if (site->receiver != NULL || site->expression != expression) { continue; }
        StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL, .environment = environment,
            .expression = expression, .site = expression->span};
        if ((site->origin != NULL && site->origin->syntax.sections.kind == MODULE_STATIC)
            || expression->selected_export.source != NULL) {
            return static_application_fail(plan, STATIC_APPLY_INVALID, expression->head,
                "Static producers and export selectors require their separate graph consumer", NULL);
        }
        if (site->statement && site->statement->kind == AST_DECLARATION) { context.declaration = site->statement; }
        if (site->root && site->context == STATIC_CONTEXT_NAMESPACE) { context.policy = STATIC_OMIT_NAMESPACE; }
        else if (site->root && context.declaration != NULL) {
            context.policy = STATIC_OMIT_FIELD;
        }
        *output = context;
        return true;
    }
    return static_application_fail(plan, STATIC_APPLY_INVALID, (SourceSpan){0},
        "Application expression does not belong to its lexical owner", NULL);
}

/** Pure prepared partial/namespace maps need no syntax. Generation always
 * uses registered sites; a supplied source policy cannot override its actual role.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) fixes lexical inference identity.
 */
bool static_application_context_valid(StaticApplicationPlan *plan, StaticApplicationContext context) {
    if (context.result != NULL && (!static_term_has_value(plan->terms, context.result) ||
        context.result->kind != STATIC_TERM_VARIABLE ||
        context.result->as.variable.role != STATIC_VARIABLE_APPLICATION ||
        context.result->as.variable.environment != context.environment ||
        context.result->as.variable.producer != context.expression)) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, context.site,
            "Application result identity does not match its lexical expression", NULL);
    }
    if (context.requests != NULL) {
        const BindingApplication *requests = plan->bindings->first_application;
        while (requests != NULL && requests != context.requests) { requests = requests->next; }
        if (requests == NULL || requests->kind != BINDING_STATIC_APPLICATION ||
            requests->expression != context.expression) {
            return static_application_fail(plan, STATIC_APPLY_INVALID, context.site,
                "Application requests do not match their original expression", NULL);
        }
    }
    if ((unsigned)context.policy > (unsigned)STATIC_OMIT_NAMESPACE) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, context.site, "Unknown omission policy", NULL);
    }
    if (context.source != NULL) {
        const StaticTermApplication *source = plan->terms->first_application;
        while (source != NULL && source != context.source) { source = source->next; }
        if (source == NULL || source->environment != context.environment ||
            source->source->expression != context.expression) {
            return static_application_fail(plan, STATIC_APPLY_INVALID, context.site,
                "Application provenance does not match its lexical expression", NULL);
        }
    }
    if (context.expression == NULL) {
        if (context.policy != STATIC_OMIT_FIELD &&
            context.declaration == NULL && (context.environment == NULL ||
            static_term_has_environment(plan->terms, context.environment))) { return true; }
    } else {
        StaticApplicationContext actual;
        if (!static_application_context(plan, context.environment, context.expression, &actual)) { return false; }
        if (actual.policy == context.policy && (context.declaration == NULL || actual.declaration == context.declaration) &&
            (context.policy != STATIC_OMIT_FIELD || context.declaration != NULL)) { return true; }
    }
    return static_application_fail(plan, STATIC_APPLY_INVALID, context.site,
        "Omission policy does not match the original source context", NULL);
}
