/**
 * @file
 * @brief Prepare local alias scope with an exact version-matched package parent.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) provides the canonical
 * version boundary; [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids
 * cross-version leakage, shadowing and local chains. [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * uses the module's containing logical directory, never its own name or caller.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"

/** Match complete canonical version bytes, not an arbitrary character prefix or physical path.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) prevents package aliases
 * leaking into another version/package or into the independent standard namespace.
 */
static bool matching_package(const ConfigurationAliasScope *package, const unsigned char *module,
    ModulePathShape shape) {
    if (shape.root == MODULE_ROOT_STANDARD) { return package == NULL; }
    return package != NULL && package->kind == CONFIGURATION_PACKAGE_SCOPE
        && package->base->root == MODULE_ROOT_PROJECT && package->base->length == shape.version_length
        && memcmp(package->base->text, module, shape.version_length) == 0;
}

ConfigurationScopeStatus configuration_module_aliases(SourceArena *arena,
    const ConfigurationDocument *document, const ConfigurationAliasScope *package,
    const unsigned char *module, size_t length, const ConfigurationAliasScope **output,
    SourceIssue *issue) {
    if (document != NULL && (!document->valid || document->profile != CONFIGURATION_MODULE)) {
        return configuration_scope_failure(issue, (SourceSpan){0}, CONFIGURATION_SCOPE_INVALID,
            "Local aliases need a valid module document", "a complete sectionless module.ini profile");
    }
    ModulePathShape shape;
    if (!module_path_describe(module, length, &shape) || shape.extent != MODULE_PATH_MODULE
        || !matching_package(package, module, shape)) {
        return configuration_scope_failure(issue, (SourceSpan){0}, CONFIGURATION_SCOPE_INVALID,
            "Module alias scope has a missing or foreign package context",
            "a complete module and its exact package-version scope, or a standard module without a package parent");
    }
    const ModulePathValue *identity = NULL;
    ModulePathStatus path_status = module_path_from_canonical(arena, module, length, &identity, issue);
    if (path_status != MODULE_PATH_OK) { return configuration_scope_path_status(path_status); }
    const ModulePathValue *base = NULL;
    path_status = module_path_parent(arena, identity, &base, issue);
    if (path_status != MODULE_PATH_OK) { return configuration_scope_path_status(path_status); }
    ConfigurationScopeWork work;
    ConfigurationScopeStatus status = configuration_scope_prepare(arena, document, true, &work, issue);
    if (status != CONFIGURATION_SCOPE_OK) { return status; }
    work.scope->kind = CONFIGURATION_MODULE_SCOPE;
    work.scope->base = base;
    work.scope->module = identity;
    work.scope->package = package;
    status = configuration_scope_keys(work, package, issue);
    if (status == CONFIGURATION_SCOPE_OK) { status = configuration_scope_targets(arena, work, document, package, issue); }
    if (status != CONFIGURATION_SCOPE_OK) { return status; }
    *output = work.scope;
    return CONFIGURATION_SCOPE_OK;
}
