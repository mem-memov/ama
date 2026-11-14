/**
 * @file
 * @brief Load one exact project closure into a single-attempt frontend owner.
 *
 * [COMP-01](../../spec/implementation/compiler.md#comp-01)
 * Project failure retains source-backed diagnostics until frontend destruction; loading does not publish artifacts.
 * [DEP-03](../../spec/implementation/compiler.md#dep-03) captures contributing
 * source/native/configuration inputs before consumers may reuse this frontend.
 */
#include <assert.h>
#include "internal.h"

/**
 * Require no prior attempt; load the complete exact project closure and retain its diagnostic on
 * failure. This function neither emits nor runs native code.
 */
bool compiler_frontend_load_required(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, const char *const *required, size_t required_count) {
    assert(!frontend->attempted);
    frontend->attempted = true;
    if (project_load_required(&frontend->project, roots, count, entry, required, required_count) != PROJECT_OK) {
        frontend->error = frontend->project.error;
        return false;
    }
    SourceStatus captured = project_inputs_capture(&frontend->inputs, &frontend->project, roots, count);
    if (captured != SOURCE_OK) {
        frontend->error = (Diagnostic){
            .code = captured == SOURCE_IO_ERROR ? DIAGNOSTIC_IO_FAILURE : DIAGNOSTIC_OUT_OF_MEMORY,
            .message = captured == SOURCE_IO_ERROR ? "A contributing native input could not be read"
                : "Compiler input tracking could not allocate storage",
            .primary = {.path = frontend->inputs.failed_path}};
        return false;
    }
    unsigned char mode = frontend->authoring ? 1 : 0;
    return compiler_frontend_input(frontend, "entry policy", &mode, 1);
}

/** Load the ordinary single-entry closure through the same exact required-module path. */
bool compiler_frontend_load(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry) {
    return compiler_frontend_load_required(frontend, roots, count, entry, NULL, 0);
}
