/** @file
 * @brief Validate and own immutable creation occurrences separately from canonical cache vertices.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains symbolic ancestry.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Exact published membership precedes dereference of a caller's path handle.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves path ownership.
 */
bool static_guard_has_path(const StaticGuardPlan *plan, const StaticCreationPath *path) {
    for (const StaticCreationPath *item = plan->first_path; item; item = item->next) {
        if (item == path) { return true; }
    }
    return false;
}

/** Repeated queries of the same occurrence retain one immutable frame. Distinct
 * source snapshots and incoming ancestries remain distinct even for equal Types.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves provenance.
 */
bool static_guard_path(StaticGuardPlan *plan, const StaticCreationPath *parent, const StaticGuardOwner *owner,
    const StaticApplicationRecord *application, SourceSpan site, const StaticCreationPath **output) {
    for (const StaticCreationPath *path = plan->first_path; path; path = path->next) {
        if (path->parent == parent && path->owner == owner && path->application == application) {
            *output = path; return true;
        }
    }
    if (plan->path_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, site,
            "Static creation path count exceeds representable storage");
    }
    StaticCreationPath *path = static_guard_allocate(plan, sizeof(*path));
    if (path == NULL) { return false; }
    *path = (StaticCreationPath){.parent = parent, .owner = owner, .application = application,
        .site = site, .next = plan->first_path};
    plan->first_path = path;
    plan->path_count++;
    *output = path;
    return true;
}
