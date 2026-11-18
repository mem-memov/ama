/** @file
 * @brief Intern original static result and alias identities in their required producer scopes.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates scoped definitions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Membership precedes source dereference. Equal names in other producer scopes
 * never import a definition or acquire an automatic dependency.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves lexical identity.
 */
bool static_guard_definition(StaticGuardPlan *plan, const StaticCreationPath *creation,
    const StaticSymbol *symbol, StaticDefinitionNode **output) {
    if (!static_guard_has_path(plan, creation) || !static_term_has_symbol(plan->applications->terms, symbol) ||
        (symbol->role != STATIC_RESULT && symbol->role != STATIC_ALIAS) ||
        symbol->owner != creation->owner->environment->definition) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0},
            "Static definition requires an original result or alias in its producer scope");
    }
    for (StaticDefinitionNode *node = plan->first_definition; node; node = node->next) {
        if (node->owner == creation->owner && node->symbol == symbol) { *output = node; return true; }
    }
    if (plan->definition_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, symbol->site,
            "Static definition count exceeds representable storage");
    }
    StaticDefinitionNode *node = static_guard_allocate(plan, sizeof(*node));
    if (node == NULL) { return false; }
    *node = (StaticDefinitionNode){.owner = creation->owner, .symbol = symbol,
        .ordinal = plan->definition_count, .next = plan->first_definition};
    node->representative = node;
    plan->first_definition = node;
    plan->definition_count++;
    *output = node;
    return true;
}
