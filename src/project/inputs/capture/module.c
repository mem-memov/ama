/**
 * @file
 * @brief Attach original module inputs and resolved dependency edges to their consumer.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) retains selected configuration
 * as a real input without rereading physical files.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) includes source, ABI and native body/header changes in project invalidation.
 */
#include "../internal.h"

/** Attach a present source/companion file; absent optional companions introduce no guessed input path. Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
static SourceStatus file(ProjectInputs *inputs, ProjectInput *module,
    const SourceText *source, const char *path) {
    if (path == NULL) { return SOURCE_OK; }
    ProjectInput *input = NULL;
    SourceStatus status = project_input_file(inputs, source, path, &input);
    return status == SOURCE_OK ? project_inputs_depend(inputs, module, input) : status;
}
/** Share the exact selected version input across its consumers without filesystem lookup.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) gives this version's
 * aliases/selectors authority; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * invalidates its consumers when the retained bytes change. Standard and unattached
 * source-only snapshots have no version record and introduce no invented file.
 */
static SourceStatus configuration(ProjectInputs *inputs, ProjectInput *consumer,
    const ModuleVersion *version) {
    if (version == NULL) { return SOURCE_OK; }
    const SourceText *source = version->configuration;
    ProjectInput *input = NULL;
    SourceStatus status = project_input_source(inputs, PROJECT_INPUT_CONFIGURATION,
        source_path(source), source, &input);
    return status == SOURCE_OK ? project_inputs_depend(inputs, consumer, input) : status;
}
SourceStatus project_input_module(ProjectInputs *inputs, const ProjectModule *module, ProjectInput *consumer) {
    const ModuleSource *source = module->source;
    SourceStatus status = file(inputs, consumer, source->text, source_path(source->text));
    if (status == SOURCE_OK) { status = configuration(inputs, consumer, source->version); }
    if (status == SOURCE_OK && source->native_abi != NULL) {
        status = file(inputs, consumer, source->native_abi, source_path(source->native_abi));
    }
    if (status == SOURCE_OK) { status = file(inputs, consumer, NULL, source->native_c); }
    if (status == SOURCE_OK) { status = file(inputs, consumer, NULL, source->native_header); }
    for (ProjectDependency *edge = module->first; status == SOURCE_OK && edge != NULL; edge = edge->next) {
        ProjectInput *target = project_inputs_find(inputs, PROJECT_INPUT_MODULE, edge->target->source->logical_path);
        status = project_inputs_depend(inputs, consumer, target);
    }
    return status;
}
