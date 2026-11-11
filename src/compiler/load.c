#include <assert.h>
#include "internal.h"

bool compiler_frontend_load(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries) {
    assert(!frontend->attempted);
    frontend->attempted = true;
    const char *roles[] = {libraries.action, libraries.index, libraries.condition, libraries.outcome};
    const char *required[4];
    size_t required_count = 0;
    for (size_t index = 0; index < 4; index++) {
        if (roles[index] != NULL) {
            required[required_count++] = roles[index];
        }
    }
    if (project_load_required(&frontend->project, roots, count, entry, required, required_count) != PROJECT_OK) {
        frontend->error = frontend->project.error;
        return false;
    }
    return true;
}
