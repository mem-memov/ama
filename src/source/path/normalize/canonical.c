/**
 * @file
 * @brief Import known canonical metadata into owned path values without invented source origins.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) gives exact
 * namespace/package structure; [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * uses canonical metadata bases separately from written paths needing reduction.
 */
#include "internal.h"
#include "source/path/logical.h"

ModulePathStatus module_path_from_canonical(SourceArena *arena, const unsigned char *bytes,
    size_t length, const ModulePathValue **output, SourceIssue *issue) {
    ModulePathShape shape;
    if (!module_path_describe(bytes, length, &shape)) {
        return path_normalization_failure(issue, (SourceSpan){0}, MODULE_PATH_INVALID,
            "Invalid canonical path metadata", "a canonical namespace prefix or complete module identity");
    }
    size_t prefix = shape.root == MODULE_ROOT_STANDARD ? 2 : 1;
    ModulePathFragment fragment = {bytes + prefix, length - prefix, {0}};
    ModulePathPart *parts;
    ModulePathStatus status = path_normalization_stack(arena, 0, fragment, &parts, issue);
    if (status != MODULE_PATH_OK) { return status; }
    size_t count = 0;
    status = path_normalization_reduce(fragment, parts, &count, issue);
    if (status != MODULE_PATH_OK) { return status; }
    return path_normalization_store(arena, shape.root, parts, count, (SourceSpan){0}, output, issue);
}
