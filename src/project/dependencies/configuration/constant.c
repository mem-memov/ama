/** @file
 * @brief Publish exact qualified constant paths only after full scoped reduction.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) fixes the final
 * component as member; [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * supplies normalized components with real or absent original provenance.
 */
#include "internal.h"

/** Retain copied member bytes and their actual provenance under the project owner.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) forbids choosing
 * another split when the exact namespace is invalid. [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * permits inherited components without fabricating source intervals.
 */
bool project_constant_path(Project *project, ProjectModule *module,
    const ModulePathValue *path, const ModuleConstantPath **output) {
    ModuleConstantPath *constant = source_arena_allocate(&project->arena, sizeof(*constant));
    if (constant == NULL) { return project_memory_failure(project); }
    SourceIssue issue;
    ModulePathStatus status = module_path_constant(&project->arena, path, constant, &issue);
    if (status == MODULE_PATH_OUT_OF_MEMORY) { return project_memory_failure(project); }
    if (status != MODULE_PATH_OK) { return project_configuration_failure(project, module, issue); }
    *output = constant;
    return true;
}
