#include <string.h>
#include "compiler/internal.h"

bool compiler_frontend_authoring(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries, const char *proposed_abi) {
    frontend->authoring = true;
    if (!compiler_frontend_load(frontend, roots, count, entry, libraries)) {
        return false;
    }
    const ProjectModule *module = frontend->project.entry;
    if (module->syntax.sections.kind != MODULE_NATIVE) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
            .message = "Native scaffolding requires an active @C module",
            .primary = {.span = {module->source->text, 0, 0}}};
        return false;
    }
    const SourceText *abi = module->source->native_abi;
    if (proposed_abi == NULL && abi == NULL) {
        proposed_abi = "+ initialize\n- destroy\n";
    }
    if (proposed_abi != NULL) {
        const char *path = source_arena_join(&frontend->project.arena,
            source_path(module->source->text), "", ".abi");
        if (path == NULL || source_manager_add(&frontend->proposed_sources, path,
                (const unsigned char *)proposed_abi, strlen(proposed_abi), &abi) != SOURCE_OK) {
            frontend->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
                .message = "Native authoring contract could not allocate storage"};
            return false;
        }
    }
    if (abi == NULL) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_SYNTAX,
            .message = "Native authoring requires an explicit proposed or existing ABI contract"};
        return false;
    }
    SemanticNativeAuthoring authoring = {module, abi};
    return compiler_frontend_finish(frontend, libraries, &authoring);
}
