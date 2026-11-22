/** @file
 * @brief Own exact absolute root keys and exclusions without inferring namespace from directory names.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires explicit configured roots;
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) rejects alias traversal;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed root scratch.
 */
#include <stdint.h>
#include "internal.h"
SourceCaptureStatus source_capture_roots(SourceRootCapture *capture, const ModuleRoot *roots, size_t count) {
    if (count > SIZE_MAX / sizeof(ModuleRoot)) { return SOURCE_CAPTURE_LIMIT; }
    ModuleRoot *owned = count ? source_arena_allocate(&capture->arena, count * sizeof(*owned)) : NULL;
    if (count && !owned) { return SOURCE_CAPTURE_MEMORY; }
    for (size_t index = 0; index < count; index++) {
        const ModuleRoot *root = &roots[index];
        if (!root->path || !*root->path || (root->kind != MODULE_ROOT_PROJECT && root->kind != MODULE_ROOT_STANDARD)
            || (root->excluded_count && !root->excluded)) { return SOURCE_CAPTURE_INVALID; }
        if (root->excluded_count > SIZE_MAX / sizeof(const char *)) { return SOURCE_CAPTURE_LIMIT; }
        SourcePhysicalPath path;
        SourcePhysicalStatus status = source_physical_resolve(&capture->arena, root->path, SOURCE_PHYSICAL_DIRECTORY,
            false, &path, &capture->root_issue);
        if (status != SOURCE_PHYSICAL_OK) {
            capture->failed_path = capture->root_issue.path; return SOURCE_CAPTURE_ROOT;
        }
        const char **excluded = root->excluded_count
            ? source_arena_allocate(&capture->arena, root->excluded_count * sizeof(*excluded)) : NULL;
        if (root->excluded_count && !excluded) { return SOURCE_CAPTURE_MEMORY; }
        for (size_t item = 0; item < root->excluded_count; item++) {
            if (!root->excluded[item]) { return SOURCE_CAPTURE_INVALID; }
            excluded[item] = source_arena_copy(&capture->arena, root->excluded[item]);
            if (!excluded[item]) { return SOURCE_CAPTURE_MEMORY; }
        }
        owned[index] = (ModuleRoot){path.path, excluded, root->excluded_count, root->kind};
    }
    capture->roots = owned; capture->root_count = count; return SOURCE_CAPTURE_OK;
}
