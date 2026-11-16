/** @file
 * @brief Reuse library declaration checks over an explicitly supplied semantic snapshot.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) requires explicit exports;
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04)
 * validates prepared settings without creating destination files or executing native code.
 */
#include "../internal.h"
/** Borrow resolved types without transferring semantic ownership or claiming a complete artifact build. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01), [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool library_configuration_validate(LibraryBuild *build, const SemanticProgram *program) {
    if (build->valid || !build->parsed || build->error.message != NULL || program == NULL
        || !program->constants_valid || program->status != SEMANTIC_OK) {
        return false;
    }
    return library_resolve_exports(build,program) && library_resolve_schemas(build,program)
        && library_resolve_bindings(build);
}
