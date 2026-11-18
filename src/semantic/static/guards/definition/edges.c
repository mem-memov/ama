/** @file
 * @brief Retain exact scoped static-definition dependency occurrences.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains scoped definition dependencies.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Equal repeated dependencies retain one source occurrence edge; different
 * original scopes and use sites remain distinct even after canonical merging.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains dependency locations.
 */
bool static_guard_definition_edge(StaticGuardPlan *plan, StaticDefinitionNode *source, StaticDefinitionNode *target,
    SourceSpan site, const StaticDefinitionEdge **output) {
    for (const StaticDefinitionEdge *item = source->first; item; item = item->next) {
        if (item->target == target && item->site.source == site.source &&
            item->site.start == site.start && item->site.end == site.end) { *output = item; return true; }
    }
    if (plan->definition_edge_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, site,
            "Static definition edge count exceeds representable storage");
    }
    StaticDefinitionEdge *item = static_guard_allocate(plan, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticDefinitionEdge){.target = target, .site = site};
    if (source->last) { source->last->next = item; }
    else { source->first = item; }
    source->last = item;
    plan->definition_edge_count++;
    *output = item;
    return true;
}
