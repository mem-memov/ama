#include <assert.h>
#include "codegen/internal.h"

bool project_codegen_build(NativeScaffold *scaffold, ScaffoldOptions options) {
    assert(!scaffold->attempted);
    scaffold->attempted = true;
    scaffold->options = options;
    if (scaffold->frontend->authoring || !scaffold_paths(scaffold) || !scaffold_model(scaffold)) { return false; }
    if (!scaffold->selected->plan->requires_pool) {
        return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "The executable root must be an active Ama or native module", NULL);
    }
    const char *const names[] = {"project.h", "project.c", "launcher.c", "compile_commands.json"};
    ScaffoldEmitter emitters[] = {codegen_emit_header, codegen_emit_body, codegen_emit_launcher, codegen_emit_tooling};
    if (!scaffold_emit_declarations(scaffold)) { return false; }
    for (size_t index = 0; index < 4; index++) {
        const char *path = scaffold_join(scaffold, scaffold->directory, "/", names[index]);
        if (!scaffold_artifact(scaffold, path, false, emitters[index], scaffold->selected)) { return false; }
    }
    scaffold->ready = true;
    return true;
}
