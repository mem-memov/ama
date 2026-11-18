/** @file
 * @brief Resolve original module requests by exact retained dependency occurrence.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) preserves every source request.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) fixes qualified namespaces.
 */
#include "../internal.h"

/** Equal path text or basenames cannot replace source-span identity. This query
 * borrows immutable module/dependency records, allocates nothing and returns null
 * for implicit, absent or differently located requests.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) supplies exact provenance.
 */
const ProjectModule *static_loaded_origin(const StaticModule *module, SourceSpan site) {
    for (const ProjectDependency *edge = module->definition->first; edge; edge = edge->next) {
        if (!edge->implicit && edge->requirement.source == site.source &&
            edge->requirement.start == site.start && edge->requirement.end == site.end) { return edge->target; }
    }
    return NULL;
}
