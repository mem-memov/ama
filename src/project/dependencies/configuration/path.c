/** @file
 * @brief Normalize an authored module or constant operand before endpoint admission.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) uses the defining
 * scope's directory and exact alias table; [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * forbids caller/table fallback. [PKG-05](../../../../spec/project/packages.md#pkg-05)
 * isolates standard origins after complete normalization.
 * [PKG-09](../../../../spec/project/packages.md#pkg-09) excludes authored src-to-tests edges.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) selects members after reduction.
 */
#include "internal.h"
#include "source/configuration/selection/selection.h"

/** Keep raw request provenance while requiring a complete normalized endpoint.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) delegates finite
 * stack reduction; [PKG-04](../../../../spec/project/packages.md#pkg-04) supplies
 * only this definition's scope. [PKG-05](../../../../spec/project/packages.md#pkg-05)
 * checks standard isolation without accepting caller package permissions.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) splits only the
 * complete normalized operand; [PKG-09](../../../../spec/project/packages.md#pkg-09)
 * keeps the resulting endpoint subject to source/test admission.
 */
bool project_dependency_path(Project *project, ProjectModule *module, SourceSpan site,
    ProjectPathRole role, ProjectPathResolution *output) {
    const ConfigurationAliasScope *scope = module->aliases;
    SourceIssue issue;
    const ModulePathValue *path = NULL;
    ModulePathStatus status = module_path_normalize(&project->arena, site, scope->base,
        configuration_alias_lookup, (void *)scope, &path, &issue);
    if (status == MODULE_PATH_OUT_OF_MEMORY) { return project_memory_failure(project); }
    if (status != MODULE_PATH_OK) { return project_configuration_failure(project, module, issue); }
    const ModuleConstantPath *constant = NULL;
    if (role == PROJECT_CONSTANT_PATH) {
        if (!project_constant_path(project, module, path, &constant)) { return false; }
        path = constant->module;
    }
    if (!project_dependency_endpoint(project, module, site, path)) { return false; }
    *output = (ProjectPathResolution){path, constant};
    return true;
}
