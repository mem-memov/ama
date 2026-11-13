/**
 * @file
 * @brief Capture one complete required closure into an explicit input graph before publication.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) shares each source identity while preserving every dependency relation.
 */
#include "../internal.h"
SourceStatus project_inputs_capture(ProjectInputs *inputs, const Project *project,
    const ModuleRoot *roots, size_t count) {
    if (inputs == NULL || project == NULL || !project->valid || inputs->sealed
        || inputs->attempted || inputs->first != NULL) { return SOURCE_INVALID_ARGUMENT; }
    inputs->attempted = true;
    for (const ProjectModule *module = project->first; module != NULL; module = module->next) {
        ProjectInput *node = NULL;
        SourceStatus status = project_inputs_put(inputs, PROJECT_INPUT_MODULE,
            module->source->logical_path, NULL, 0, &node);
        if (status != SOURCE_OK) { return status; }
    }
    ProjectInput *configuration = NULL;
    SourceStatus status = project_input_roots(inputs, roots, count, &configuration);
    for (const ProjectModule *module = project->first; status == SOURCE_OK && module != NULL; module = module->next) {
        ProjectInput *node = project_inputs_find(inputs, PROJECT_INPUT_MODULE, module->source->logical_path);
        status = project_inputs_depend(inputs, node, configuration);
        if (status == SOURCE_OK) { status = project_input_module(inputs, module, node); }
    }
    return status;
}
