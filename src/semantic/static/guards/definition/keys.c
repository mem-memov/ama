/** @file
 * @brief Merge static-definition identities only after exact complete owner normalization.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves scoped symbolic definitions.
 */
#include "../internal.h"

/** Unresolved owner keys retain original scope. A closed owner/symbol pair
 * chooses its least original node ordinal only as a representative; all nodes
 * and incoming/source edges remain stored and are traversed by the checker.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves merged dependencies.
 */
bool static_guard_definition_keys(StaticGuardPlan *plan, StaticGuardClosure *closure) {
    for (StaticDefinitionNode *node = plan->first_definition; node; node = node->next) {
        const StaticTerm *value;
        StaticConstraintOutcome outcome;
        if (!static_constraint_value(plan->applications->solver, node->owner->value, &value, &outcome)) {
            (void)static_guard_dependencies(plan); return false;
        }
        if (outcome == STATIC_CONSTRAINT_PENDING) { *closure = STATIC_GUARD_PENDING; continue; }
        if (node->concrete != NULL && node->concrete != value) {
            return static_guard_fail(plan, STATIC_GUARD_CONFLICT, node->symbol->site,
                "A complete static-definition owner changed after normalization");
        }
        node->concrete = value;
    }
    for (StaticDefinitionNode *node = plan->first_definition; node; node = node->next) {
        node->representative = node;
        if (node->concrete == NULL) { continue; }
        for (StaticDefinitionNode *other = plan->first_definition; other; other = other->next) {
            if (other->symbol == node->symbol && other->concrete == node->concrete &&
                other->ordinal < node->representative->ordinal) { node->representative = other; }
        }
    }
    return true;
}
