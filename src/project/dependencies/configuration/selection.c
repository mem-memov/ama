/** @file
 * @brief Compile one owning version's dependency masks against actual prepared availability.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) unions actual matches;
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) uses only package aliases.
 */
#include <stdint.h>
#include "internal.h"

/** Compile complete masks before publishing their exact selected-root union.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) does not load targets or
 * synthesize wildcard versions; [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * supplies the owning alias scope. Checked finite scans retain all partial storage.
 */
bool project_package_selection(Project *project, ProjectModule *module,
    const ConfigurationDocument *document, const ConfigurationAliasScope *scope,
    const DependencySelection **output) {
    if (!project_configuration_view(project, module)) { return false; }
    size_t count = 0;
    for (const ConfigurationSelector *item = document->selectors; item != NULL; item = item->next) {
        if (count == SIZE_MAX / sizeof(DependencySelector *)) { return project_memory_failure(project); }
        count++;
    }
    const DependencySelector **patterns = count == 0 ? NULL : source_arena_allocate(&project->arena, count * sizeof(*patterns));
    if (count != 0 && patterns == NULL) { return project_memory_failure(project); }
    size_t index = 0;
    SourceIssue issue;
    for (const ConfigurationSelector *item = document->selectors; item != NULL; item = item->next) {
        DependencySelectorStatus status = dependency_selector_compile(&project->arena, item->pattern,
            scope, &patterns[index++], &issue);
        if (status == DEPENDENCY_SELECTOR_OUT_OF_MEMORY) { return project_memory_failure(project); }
        if (status != DEPENDENCY_SELECTOR_OK) { return project_configuration_failure(project, module, issue); }
    }
    DependencySelectionStatus status = dependency_selection_build(&project->arena,
        project->dependency_view, scope->base, patterns, count, output, &issue);
    if (status == DEPENDENCY_SELECTION_OUT_OF_MEMORY) { return project_memory_failure(project); }
    return status == DEPENDENCY_SELECTION_OK || project_configuration_failure(project, module, issue);
}
