/** @file
 * @brief Track the library configuration, closure and explicit foreign link-file bytes.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) invalidates the
 * component consumer from exact dependency changes without rewriting any source.
 */
#include <string.h>
#include "../internal.h"
/** Retain an exact input-tracking failure; source reads preserve their physical path. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
static bool input_failure(LibraryBuild *build, SourceStatus status, const char *path) {
    build->error = (Diagnostic){.code = status == SOURCE_IO_ERROR ? DIAGNOSTIC_IO_FAILURE : DIAGNOSTIC_OUT_OF_MEMORY,
        .message = "Library build inputs could not be captured", .primary = {.path = path}};
    return false;
}
bool library_capture_inputs(LibraryBuild *build) {
    ProjectInputs *inputs = &build->frontend.inputs;
    ProjectInput *consumer = NULL, *configuration = NULL;
    SourceStatus status = project_inputs_put(inputs, PROJECT_INPUT_GENERATED, "Ama C library component",
        (const unsigned char *)build->component, strlen(build->component), &consumer);
    if (status == SOURCE_OK) {
        status = project_inputs_put(inputs, PROJECT_INPUT_CONFIGURATION, "Ama C library configuration",
            source_bytes(build->configuration), source_length(build->configuration), &configuration);
    }
    if (status == SOURCE_OK) { status = project_inputs_depend(inputs, consumer, configuration); }
    if (status != SOURCE_OK) { return input_failure(build, status, NULL); }
    for (const ProjectModule *module = build->frontend.project.first; module != NULL; module = module->next) {
        ProjectInput *dependency = project_inputs_find(inputs, PROJECT_INPUT_MODULE, module->source->logical_path);
        status = project_inputs_depend(inputs, consumer, dependency);
        if (status != SOURCE_OK) { return input_failure(build, status, NULL); }
    }
    for (const LibrarySetting *setting = build->settings; setting != NULL; setting = setting->next) {
        if (strcmp(setting->kind, "link-file") != 0 && strcmp(setting->kind, "dependency-file") != 0) { continue; }
        ProjectInput *dependency = project_inputs_find(inputs, PROJECT_INPUT_FILE, setting->value);
        if (dependency == NULL) {
            const SourceText *source = NULL;
            status = source_manager_load(&build->sources, setting->value, &source);
            if (status != SOURCE_OK) { return input_failure(build, status, setting->value); }
            status = project_inputs_put(inputs, PROJECT_INPUT_FILE, setting->value,
                source_bytes(source), source_length(source), &dependency);
        }
        if (status == SOURCE_OK) { status = project_inputs_depend(inputs, consumer, dependency); }
        if (status != SOURCE_OK) { return input_failure(build, status, setting->value); }
    }
    return true;
}
