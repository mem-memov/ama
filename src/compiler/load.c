#include <assert.h>
#include "internal.h"

bool compiler_frontend_load(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry) {
    assert(!frontend->attempted);
    frontend->attempted = true;
    if (project_load(&frontend->project, roots, count, entry) != PROJECT_OK) {
        frontend->error = frontend->project.error;
        return false;
    }
    return true;
}
