/** @file
 * @brief Own incoming static-definition expansion paths without losing source ancestry.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains scoped definition dependencies.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Preserve each incoming active ancestry, including roots from different
 * creation occurrences whose source definition node is shared.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains expansion provenance.
 */
bool static_guard_definition_path(StaticGuardPlan *plan, const StaticDefinitionPath *parent,
    const StaticCreationPath *creation, StaticDefinitionNode *node,
    const StaticDefinitionEdge *incoming, SourceSpan site, const StaticDefinitionPath **output) {
    for (const StaticDefinitionPath *item = plan->first_definition_path; item; item = item->next) {
        if (item->parent == parent && item->creation == creation && item->node == node && item->edge == incoming) {
            *output = item; return true;
        }
    }
    if (plan->definition_path_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, site,
            "Static definition path count exceeds representable storage");
    }
    StaticDefinitionPath *item = static_guard_allocate(plan, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticDefinitionPath){.parent = parent, .creation = creation, .node = node,
        .edge = incoming, .site = site, .next = plan->first_definition_path};
    plan->first_definition_path = item;
    plan->definition_path_count++;
    *output = item;
    return true;
}
