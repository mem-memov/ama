/** @file
 * @brief Prepare aliases in the lexical definition's retained configuration environment.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids caller/submodule
 * inheritance and shadowing. [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * separates sectionless local aliases from package profiles.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"

/** Parse an optional captured local document, then publish only a complete defining scope.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) uses the containing logical
 * directory and at most one package parent; [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * distinguishes absent configuration from a supplied empty document.
 */
bool project_prepare_aliases(Project *project, ProjectModule *module) {
    const char *identity = module->source->logical_path;
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)) {
        return project_configuration_failure(project, module,
            (SourceIssue){{module->source->text, 0, 0}, "Invalid retained module identity", "canonical module identity"});
    }
    const ConfigurationAliasScope *package = NULL;
    if (shape.root == MODULE_ROOT_PROJECT && !project_package_aliases(project, module, &package, &module->dependencies)) { return false; }
    ConfigurationDocument document;
    configuration_document_init(&document, project->arena.allocator);
    ConfigurationStatus parsed = CONFIGURATION_OK;
    bool present = module->source->configuration != NULL;
    if (present) { parsed = configuration_document_parse(&document, module->source->configuration, CONFIGURATION_MODULE); }
    SourceIssue issue = document.issue;
    ConfigurationScopeStatus prepared = CONFIGURATION_SCOPE_INVALID;
    if (parsed == CONFIGURATION_OK) {
        prepared = configuration_module_aliases(&project->arena, present ? &document : NULL,
            package, (const unsigned char *)identity, strlen(identity), &module->aliases, &issue);
    }
    configuration_document_destroy(&document);
    if (parsed == CONFIGURATION_OUT_OF_MEMORY || prepared == CONFIGURATION_SCOPE_OUT_OF_MEMORY) {
        return project_memory_failure(project);
    }
    return (parsed == CONFIGURATION_OK && prepared == CONFIGURATION_SCOPE_OK)
        || project_configuration_failure(project, module, issue);
}
