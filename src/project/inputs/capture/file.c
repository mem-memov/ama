/**
 * @file
 * @brief Reuse retained source text and acquire each additional native file once.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) tracks native companions as real inputs without reparsing them as Ama.
 */
#include "../internal.h"
SourceStatus project_input_file(ProjectInputs *inputs, const SourceText *source,
    const char *path, ProjectInput **output) {
    ProjectInput *existing = project_inputs_find(inputs, PROJECT_INPUT_FILE, path);
    if (existing != NULL) {
        if (source != NULL) { return project_input_source(inputs, PROJECT_INPUT_FILE, path, source, output); }
        *output = existing;
        return SOURCE_OK;
    }
    if (source == NULL) {
        const char *owned_path = source_arena_copy(&inputs->arena, path);
        if (owned_path == NULL) { return SOURCE_OUT_OF_MEMORY; }
        SourceStatus status = source_manager_load(&inputs->sources, owned_path, &source);
        if (status != SOURCE_OK) {
            inputs->failed_path = owned_path;
            return status;
        }
    }
    return project_input_source(inputs, PROJECT_INPUT_FILE, path, source, output);
}
