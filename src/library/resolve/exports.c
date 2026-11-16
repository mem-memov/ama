/** @file
 * @brief Match only declared exports to executable complete nominal module types.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) forbids
 * exporting internals by discovery or permitting entry on constants-only modules.
 */
#include "../internal.h"
bool library_resolve_exports(LibraryBuild *build, const SemanticProgram *program) {
    for (LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        exported->type = semantic_find_type(program, exported->module);
        if (exported->type == NULL || !exported->type->executable) {
            return library_fail(build, exported->site, "A library export must name an available executable module type");
        }
    }
    return true;
}
