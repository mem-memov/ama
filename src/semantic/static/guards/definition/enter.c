/** @file
 * @brief Guard active and cached static-definition dependencies before exposing their expansion frames.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) rejects directed definition cycles.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains original chain evidence.
 */
#include "../internal.h"

/** Parent definition handles are exact published occurrences, not copies or
 * nodes from another producer/application owner.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) scopes dependencies.
 */
static bool parent_valid(const StaticGuardPlan *plan, const StaticDefinitionPath *parent, SourceSpan site) {
    if (parent == NULL) { return true; }
    for (const StaticDefinitionPath *path = plan->first_definition_path; path; path = path->next) {
        if (path != parent) { continue; }
        return site.source == path->node->owner->environment->definition->definition->source->text && source_span_valid(site);
    }
    return false;
}

/** Canonical node representatives account for already closed equal scopes;
 * unresolved distinct scopes are retained for later whole-graph rechecking.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) checks active definitions.
 */
static bool active(StaticGuardPlan *plan, const StaticDefinitionPath *parent,
    StaticDefinitionNode *node, SourceSpan site) {
    for (const StaticDefinitionPath *ancestor = parent; ancestor; ancestor = ancestor->parent) {
        if (ancestor->node->representative != node->representative) { continue; }
        plan->rejected_definition_path = parent;
        plan->repeated_definition_ancestor = ancestor;
        plan->rejected_definition = node;
        plan->rejected_site = site;
        (void)static_guard_fail(plan, STATIC_GUARD_CONFLICT, site, "Cyclic static value-definition expansion");
        const char *name = node->symbol->owner->definition->source->logical_path;
        plan->related[0] = (DiagnosticRelated){.location = {.span = node->symbol->site},
            .role = "repeated static definition", .name = name};
        plan->related[1] = (DiagnosticRelated){.location = {.span = ancestor->site},
            .role = "earlier definition expansion", .name = name};
        plan->error.related = plan->related;
        plan->error.related_count = 2;
        return false;
    }
    return true;
}

/** Interleave complete-key refresh and edge publication so neither cached
 * dependencies nor newly equal producer scopes bypass active cycle checks.
 * Root sites are original symbol introductions; dependency sites belong to the
 * source parent definition's original file, including captured foreign aliases.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) guards before following edges.
 */
bool static_definition_enter(StaticGuardPlan *plan, const StaticDefinitionPath *parent,
    const StaticCreationPath *creation, const StaticSymbol *symbol, SourceSpan site,
    const StaticDefinitionPath **output) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (output == NULL || !parent_valid(plan, parent, site)) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0},
            "Invalid static definition parent, original dependency site or receiver");
    }
    StaticDefinitionNode *node;
    if (!static_guard_definition(plan, creation, symbol, &node)) { return false; }
    if (parent == NULL) { site = symbol->site; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure) || !active(plan, parent, node, site)) { return false; }
    const StaticDefinitionEdge *incoming = NULL;
    if (parent != NULL && !static_guard_definition_edge(plan, parent->node, node, site, &incoming)) { return false; }
    const StaticDefinitionPath *prepared;
    if (!static_guard_definition_path(plan, parent, creation, node, incoming, site, &prepared) ||
        !static_guard_refresh(plan, &closure)) { return false; }
    *output = prepared;
    return true;
}
