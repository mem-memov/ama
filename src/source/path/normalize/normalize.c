/**
 * @file
 * @brief Compose raw admission, one alias lookup, stack reduction and immutable result publication.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) preserves the
 * defining directory's root for relative paths; [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * makes an alias target's already normalized root authoritative.
 */
#include "internal.h"

ModulePathStatus module_path_normalize(SourceArena *arena, SourceSpan expression,
    const ModulePathValue *base, ModulePathAliasLookup lookup, void *context,
    const ModulePathValue **output, SourceIssue *issue) {
    ModulePathInput input;
    ModulePathStatus status = path_normalization_read(expression, &input, issue);
    if (status != MODULE_PATH_OK) { return status; }
    const ModulePathValue *prefix = NULL;
    if (input.aliased) {
        prefix = lookup == NULL ? NULL : lookup(context, input.alias);
        if (prefix == NULL) {
            return path_normalization_failure(issue, input.alias, MODULE_PATH_UNKNOWN_ALIAS,
                "Unknown path alias", "one complete alias visible in the defining scope");
        }
    } else if (input.relative) {
        prefix = base;
        if (prefix == NULL) {
            return path_normalization_failure(issue, expression, MODULE_PATH_MISSING_BASE,
                "Relative path has no defining directory", "an explicit defining-directory base");
        }
    }
    ModuleRootKind root = prefix == NULL ? input.root : prefix->root;
    size_t count = prefix == NULL ? 0 : prefix->count;
    ModulePathPart *parts;
    status = path_normalization_stack(arena, count, input.fragment, &parts, issue);
    if (status != MODULE_PATH_OK) { return status; }
    for (size_t index = 0; index < count; index++) { parts[index] = prefix->parts[index]; }
    status = path_normalization_reduce(input.fragment, parts, &count, issue);
    if (status != MODULE_PATH_OK) { return status; }
    return path_normalization_store(arena, root, parts, count, expression, output, issue);
}
