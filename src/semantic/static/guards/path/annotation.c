/** @file
 * @brief Retain original annotation owners as graph predecessors without inventing executable roots.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) attributes every
 * required application to its lexical owner and forbids arbitrary new roots.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps
 * partially prepared provenance under the guard's existing arena ownership.
 */
#include "../internal.h"

/** Original root occurrence identity excludes copied syntax, nested operands
 * presented as annotations and static graph statements. These checks inspect
 * only already accepted inventory; no definition or body is expanded.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) retains scope.
 */
static bool original(const StaticTermEnvironment *environment, const AstTypeExpression *expression) {
    if (environment->definition->definition->syntax.sections.kind == MODULE_STATIC) { return false; }
    for (const StaticOccurrence *site = environment->definition->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->root && site->expression == expression) { return true; }
    }
    return false;
}

/** The explicit source-only bit prevents a source fact from acquiring registry
 * admission when a child producer is checked. Exact owner keys may still join
 * the ordinary creation graph for ancestry validation; they certify no body.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves both
 * source equations and independently required actual incoming checks.
 */
bool static_creation_annotation(StaticGuardPlan *plan, const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, const StaticCreationPath **output) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (!output || !static_term_has_environment(plan->applications->terms, environment) ||
        !original(environment, expression)) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Invalid original ordinary annotation provenance");
    }
    const StaticGuardOwner *owner;
    const StaticCreationPath *path;
    if (!static_guard_owner(plan, environment, &owner) ||
        !static_guard_path(plan, NULL, owner, NULL, expression->span, true, &path)) { return false; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    *output = path;
    return true;
}
