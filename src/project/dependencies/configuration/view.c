/** @file
 * @brief Cache neutral selected-catalog availability and translate failures into Project diagnostics.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) uses the shared actual
 * universe; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * successful and failed preparation in the Project arena without duplicate owners.
 */
#include "internal.h"
#include "source/configuration/selection/catalog.h"
/** Cache once using the actual universe required by
 * [PKG-05](../../../../spec/project/packages.md#pkg-05); no independent discovery,
 * guessed ancestry or selector expansion is introduced by the Project adapter.
 */
bool project_configuration_view(Project *project, ProjectModule *module) {
    if (project->dependency_view) { return true; }
    SourceIssue issue = {0};
    DependencySelectionStatus status = dependency_catalog_view(&project->arena, &project->catalog,
        &project->dependency_view, &issue);
    if (status == DEPENDENCY_SELECTION_OUT_OF_MEMORY || status == DEPENDENCY_SELECTION_LIMIT) { return project_memory_failure(project); }
    return status == DEPENDENCY_SELECTION_OK || project_configuration_failure(project, module, issue);
}
