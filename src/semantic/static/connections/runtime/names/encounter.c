/** @file
 * @brief Retain the exact set of module-variable base names consumed by the lexical prefix.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) decides receiver freshness independently of Types.
 */
#include <stdint.h>
#include "../../internal.h"

/** Complete name-byte equality is the only set membership predicate. Locals and
 * foreign names are excluded by the caller; allocation precedes publication.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) treats multiword names as indivisible.
 */
bool static_runtime_encounter(StaticConnectionPlan *plan, StaticRuntimeModule *module, SourceSpan name, bool *earlier) {
    for (const StaticRuntimeEncounter *seen = module->encountered; seen; seen = seen->next) {
        if (static_same_name(seen->name, name)) { if (earlier) { *earlier = true; } return true; }
    }
    if (module->encounter_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, name, "Runtime encounter count exceeds representable storage");
    }
    StaticRuntimeEncounter *seen = static_connection_allocate(plan, sizeof(*seen));
    if (seen == NULL) { return false; }
    *seen = (StaticRuntimeEncounter){.next = module->encountered, .name = name};
    module->encountered = seen;
    module->encounter_count++;
    if (earlier) { *earlier = false; }
    return true;
}
