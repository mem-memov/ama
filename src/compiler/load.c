/**
 * @file
 * @brief Load one exact project closure into a single-attempt frontend owner.
 *
 * [COMP-01](../../spec/implementation/compiler.md#comp-01)
 * Project failure retains source-backed diagnostics until frontend destruction; loading does not publish artifacts.
 */
#include <assert.h>
#include "internal.h"

/**
 * Require no prior attempt; load the complete exact project closure and retain its diagnostic on
 * failure. This function neither emits nor runs native code.
 */
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
