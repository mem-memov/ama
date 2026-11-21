/** @file
 * @brief Check a normalized authored module endpoint in its defining package.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) checks selected external
 * permissions and isolates standard definitions; [PKG-09](../../../../spec/project/packages.md#pkg-09)
 * forbids explicit source-to-test edges after normalization.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) requires a complete endpoint.
 */
#include "internal.h"

/** Admit one complete namespace/module identity without changing its raw source site.
 * Missing actual targets remain missing-module errors even if a mask selects no
 * identities. Standard callers never borrow an ordinary package environment.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05),
 * [PKG-09](../../../../spec/project/packages.md#pkg-09) and
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) fix these checks.
 */
bool project_dependency_endpoint(Project *project, ProjectModule *module,
    SourceSpan site, const ModulePathValue *path) {
    const ConfigurationAliasScope *scope = module->aliases;
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)path->text, path->length, &shape)
        || shape.extent != MODULE_PATH_MODULE) {
        return project_configuration_failure(project, module,
            (SourceIssue){site, "Dependency path does not select a complete module", "one canonical module identity"});
    }
    if (scope->module->root == MODULE_ROOT_STANDARD
        && dependency_standard_edge_check(scope->module, path) != DEPENDENCY_EDGE_ALLOWED) {
        return project_configuration_failure(project, module,
            (SourceIssue){site, "A standard module cannot explicitly depend on an ordinary module",
                "a standard module or a supplied Type/Template parameter"});
    }
    if (scope->module->root == MODULE_ROOT_PROJECT) {
        DependencyEdgeDecision decision = dependency_edge_check(module->dependencies, scope->module, path);
        if (decision != DEPENDENCY_EDGE_SOURCE_TEST) {
            const ModuleSource *available = NULL;
            if (module_catalog_find(&project->catalog, path->text, &available) == MODULE_MISSING) {
                return project_report(project, PROJECT_MISSING, module, site, path->text, NULL);
            }
        }
        if (decision != DEPENDENCY_EDGE_ALLOWED) {
            return project_configuration_failure(project, module, (SourceIssue){site,
                decision == DEPENDENCY_EDGE_SOURCE_TEST
                    ? "A source definition cannot explicitly depend on tests"
                    : "External dependency is not covered by this version's selectors",
                "an allowed dependency in the defining package configuration"});
        }
    }
    return true;
}
