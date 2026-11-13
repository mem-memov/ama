/**
 * @file
 * @brief Attach frontend policy and authoring configuration to the exact entry consumer.
 * [DEP-03](../../spec/implementation/compiler.md#dep-03) requires configuration
 * inputs to remain distinct from loaded source bytes and generated artifacts.
 */
#include "internal.h"
bool compiler_frontend_input(CompilerFrontend *frontend, const char *key,
    const unsigned char *bytes, size_t length) {
    ProjectInput *input = NULL;
    SourceStatus status = project_inputs_put(&frontend->inputs, PROJECT_INPUT_CONFIGURATION,
        key, bytes, length, &input);
    if (status == SOURCE_OK) {
        ProjectInput *entry = project_inputs_find(&frontend->inputs, PROJECT_INPUT_MODULE,
            frontend->project.entry->source->logical_path);
        status = project_inputs_depend(&frontend->inputs, entry, input);
    }
    if (status != SOURCE_OK) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Frontend configuration tracking could not allocate storage"};
        return false;
    }
    return true;
}
