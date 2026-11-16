/** @file
 * @brief Compare complete before/final source registries under the exact physical move.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * includes unused importers; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires a defined injective identity mapping for every original source.
 */
#include "internal.h"

/** Original parsing precedes projected discovery; no original candidate may disappear behind an exclusion or invalid namespace. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_discover(RelocationPreview *preview) {
    if (!relocation_original_modules(preview)) { return false; }
    RelocationDirectoryIdentity *identities = NULL;
    for (size_t index = 0; index < preview->after_root_count; index++) {
        if (!relocation_walk_root(preview,&preview->after_roots[index],&identities)) { return false; }
    }
    for (const RelocationModule *module = preview->modules; module != NULL; module = module->next) {
        if (module->after_identity == NULL) { return relocation_fail(preview,RELOCATION_UNCOVERED,module->before_path,NULL); }
    }
    return true;
}
