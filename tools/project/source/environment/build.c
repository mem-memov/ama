/** @file
 * @brief Join actual catalog availability with every retained package's own strict scope.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires original and final selection facts; [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * forbids malformed configuration fallback, including in empty actual versions.
 */
#include "internal.h"
#include <string.h>
/** Register initialized rows before parsing so every failure path has one owner. */
static SourceEnvironmentStatus package(SourcePackageEnvironment *owner, const ModuleVersion *version) {
    owner->failed_version = version; owner->phase = SOURCE_ENVIRONMENT_DOCUMENT;
    SourcePackageScope *row = source_arena_allocate(&owner->arena, sizeof(*row));
    if (!row) { return SOURCE_ENVIRONMENT_MEMORY; }
    *row = (SourcePackageScope){.version = version};
    configuration_document_init(&row->document, owner->arena.allocator);
    if (owner->last) { owner->last->next = row; } else { owner->first = row; } owner->last = row;
    ConfigurationStatus parsed = configuration_document_parse(&row->document, version->configuration, CONFIGURATION_PACKAGE);
    owner->issue = row->document.issue;
    if (parsed == CONFIGURATION_OUT_OF_MEMORY) { return SOURCE_ENVIRONMENT_MEMORY; }
    if (parsed != CONFIGURATION_OK) { return SOURCE_ENVIRONMENT_CONFIGURATION; }
    owner->phase = SOURCE_ENVIRONMENT_ALIASES;
    ConfigurationScopeStatus status = configuration_package_aliases(&owner->arena, &row->document,
        (const unsigned char *)version->logical_path, strlen(version->logical_path), &row->aliases, &owner->issue);
    if (status == CONFIGURATION_SCOPE_OUT_OF_MEMORY) { return SOURCE_ENVIRONMENT_MEMORY; }
    if (status == CONFIGURATION_SCOPE_LIMIT) { return SOURCE_ENVIRONMENT_LIMIT; }
    return status == CONFIGURATION_SCOPE_OK ? source_environment_select(owner, row) : SOURCE_ENVIRONMENT_CONFIGURATION;
}
SourceEnvironmentStatus source_package_environment_build(SourcePackageEnvironment *owner, const ModuleCatalog *catalog) {
    if (!owner || owner->attempted) { return SOURCE_ENVIRONMENT_INVALID; }
    owner->attempted = true;
    if (!catalog || !catalog->built) { return owner->status = SOURCE_ENVIRONMENT_INVALID; }
    owner->catalog = catalog; owner->phase = SOURCE_ENVIRONMENT_VIEW;
    DependencySelectionStatus status = dependency_catalog_view(&owner->arena, catalog, &owner->view, &owner->issue);
    if (status != DEPENDENCY_SELECTION_OK) {
        return owner->status = status == DEPENDENCY_SELECTION_OUT_OF_MEMORY ? SOURCE_ENVIRONMENT_MEMORY
            : status == DEPENDENCY_SELECTION_LIMIT ? SOURCE_ENVIRONMENT_LIMIT : SOURCE_ENVIRONMENT_CONFIGURATION;
    }
    for (const ModuleVersion *version = catalog->versions; version; version = version->next) {
        owner->status = package(owner, version);
        if (owner->status != SOURCE_ENVIRONMENT_OK) { return owner->status; }
    }
    owner->failed_version = NULL; owner->issue = (SourceIssue){0}; owner->valid = true;
    return SOURCE_ENVIRONMENT_OK;
}
