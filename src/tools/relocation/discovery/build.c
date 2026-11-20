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
    ModulePackage *packages = NULL;
    if (!relocation_select_packages(preview, &packages)) { return false; }
    for (size_t index = 0; index < preview->after_root_count; index++) {
        const ModuleRoot *root = &preview->after_roots[index];
        if (root->kind == MODULE_ROOT_STANDARD && !relocation_walk_root(preview, root)) { return false; }
    }
    for (const ModulePackage *package = packages; package != NULL; package = package->next) {
        if (!relocation_walk_package(preview, package)) { return false; }
    }
    for (const RelocationModule *module = preview->modules; module != NULL; module = module->next) {
        if (module->after_identity == NULL) { return relocation_fail(preview,RELOCATION_UNCOVERED,module->before_path,NULL); }
    }
    return true;
}
