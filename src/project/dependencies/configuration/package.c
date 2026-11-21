/** @file
 * @brief Parse and cache one required retained package's immutable alias environment.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) defines strict INI inputs;
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) shares an exact version scope.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) caches exact dependency coverage.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed scratch.
 */
#include <string.h>
#include "internal.h"

/** Complete transient document parsing and scope construction before cache publication.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) keys ownership by the
 * selected version, while [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * retains original bytes in the catalog after transient records are destroyed.
 * Exact cache scans terminate at their finite list end; no scope is fabricated.
 */
bool project_package_aliases(Project *project, ProjectModule *module,
    const ConfigurationAliasScope **output, const DependencySelection **selection) {
    const ModuleVersion *version = module->source->version;
    if (version == NULL) {
        SourceIssue issue = {{module->source->text, 0, 0},
            "Ordinary module snapshot lacks its selected package configuration", "explicit retained version context"};
        return project_configuration_failure(project, module, issue);
    }
    for (ProjectAliasPackage *entry = project->alias_packages; entry != NULL; entry = entry->next) {
        if (entry->version == version) {
            *output = entry->scope;
            *selection = entry->selection;
            return true;
        }
    }
    ConfigurationDocument document;
    configuration_document_init(&document, project->arena.allocator);
    ConfigurationStatus parsed = configuration_document_parse(&document, version->configuration, CONFIGURATION_PACKAGE);
    SourceIssue issue = document.issue;
    const ConfigurationAliasScope *scope = NULL;
    ConfigurationScopeStatus prepared = CONFIGURATION_SCOPE_INVALID;
    if (parsed == CONFIGURATION_OK) {
        prepared = configuration_package_aliases(&project->arena, &document,
            (const unsigned char *)version->logical_path, strlen(version->logical_path), &scope, &issue);
    }
    const DependencySelection *selected = NULL;
    bool selected_ok = true;
    if (parsed == CONFIGURATION_OK && prepared == CONFIGURATION_SCOPE_OK) {
        selected_ok = project_package_selection(project, module, &document, scope, &selected);
    }
    configuration_document_destroy(&document);
    if (!selected_ok) { return false; }
    if (parsed == CONFIGURATION_OUT_OF_MEMORY || prepared == CONFIGURATION_SCOPE_OUT_OF_MEMORY) {
        return project_memory_failure(project);
    }
    if (parsed != CONFIGURATION_OK || prepared != CONFIGURATION_SCOPE_OK) {
        return project_configuration_failure(project, module, issue);
    }
    ProjectAliasPackage *entry = source_arena_allocate(&project->arena, sizeof(*entry));
    if (entry == NULL) { return project_memory_failure(project); }
    *entry = (ProjectAliasPackage){project->alias_packages, version, scope, selected};
    project->alias_packages = entry;
    *output = scope;
    *selection = selected;
    return true;
}
