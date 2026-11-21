/**
 * @file
 * @brief Prepare one package-version alias environment before publishing any entry.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) supplies a real canonical
 * version base; [PKG-04](../../../../spec/project/packages.md#pkg-04) permits only
 * direct targets and defines this base independently of any consuming module.
 */
#include "internal.h"
#include "source/path/logical.h"

ConfigurationScopeStatus configuration_package_aliases(SourceArena *arena,
    const ConfigurationDocument *document, const unsigned char *version, size_t length,
    const ConfigurationAliasScope **output, SourceIssue *issue) {
    if (document == NULL || !document->valid || document->profile != CONFIGURATION_PACKAGE) {
        return configuration_scope_failure(issue, (SourceSpan){0}, CONFIGURATION_SCOPE_INVALID,
            "Package alias scope needs a valid package document", "a complete package.ini profile");
    }
    ModulePathShape shape;
    if (!module_path_describe(version, length, &shape) || shape.root != MODULE_ROOT_PROJECT
        || shape.extent != MODULE_PATH_VERSION) {
        return configuration_scope_failure(issue, (SourceSpan){0}, CONFIGURATION_SCOPE_INVALID,
            "Package alias base is not a version identity", "one canonical ordinary package-version identity");
    }
    const ModulePathValue *base = NULL;
    ModulePathStatus path_status = module_path_from_canonical(arena, version, length, &base, issue);
    if (path_status != MODULE_PATH_OK) { return configuration_scope_path_status(path_status); }
    ConfigurationScopeWork work;
    ConfigurationScopeStatus status = configuration_scope_prepare(arena, document, false, &work, issue);
    if (status != CONFIGURATION_SCOPE_OK) { return status; }
    work.scope->kind = CONFIGURATION_PACKAGE_SCOPE;
    work.scope->base = base;
    status = configuration_scope_keys(work, NULL, issue);
    if (status == CONFIGURATION_SCOPE_OK) { status = configuration_scope_targets(arena, work, document, NULL, issue); }
    if (status != CONFIGURATION_SCOPE_OK) { return status; }
    *output = work.scope;
    return CONFIGURATION_SCOPE_OK;
}
