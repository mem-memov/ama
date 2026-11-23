/** @file
 * @brief Decide original-analysis coverage by exact physical subtree/exclusion boundaries.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires newly covered sources in the original analysis; observed physical
 * ancestors alone do not provide semantic coverage under
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
#include "internal.h"
#include <string.h>
/** Membership is constant between the finite root/exclusion boundary prefixes. */
static bool includes(const ModuleRoot *root, const char *path) {
    if (!source_image_below(path, root->path)) { return false; }
    const char *relative = path + strlen(root->path);
    if (*relative == '/') { relative++; }
    return !module_path_excluded(root, relative);
}
/** A point belongs to the union only under the requested namespace family. */
static bool covered(const ModuleRoot *roots, size_t count, const ModuleRoot *wanted, const char *path) {
    for (size_t i = 0; i < count; i++) {
        if (roots[i].kind == wanted->kind && includes(&roots[i], path)) { return true; }
    }
    return false;
}
/** Check a complete region symbolically. Any first uncovered region begins at
 * the requested root, a provider root or an exclusion endpoint. Evaluating all
 * endpoints is exact even when nested providers fill each other's exclusions;
 * enumerating currently existing files would miss future/unobserved sources. */
SourceOperationRootsStatus source_operation_root_region(SourceArena *arena, const ModuleRoot *wanted,
    const ModuleRoot *provided, size_t count, const char **failed_path) {
    *failed_path = wanted->path;
    if (includes(wanted, wanted->path) && !covered(provided, count, wanted, wanted->path)) { return SOURCE_OPERATION_ROOTS_COVERAGE; }
    for (size_t i = 0; i < count; i++) {
        const ModuleRoot *root = &provided[i];
        if (includes(wanted, root->path) && !covered(provided, count, wanted, root->path)) {
            *failed_path = root->path; return SOURCE_OPERATION_ROOTS_COVERAGE;
        }
        for (size_t j = 0; j < root->excluded_count; j++) {
            const char *boundary = source_arena_join(arena, root->path,
                root->path[strlen(root->path)-1] == '/' ? "" : "/", root->excluded[j]);
            if (!boundary) { return SOURCE_OPERATION_ROOTS_MEMORY; }
            if (includes(wanted, boundary) && !covered(provided, count, wanted, boundary)) {
                *failed_path = boundary; return SOURCE_OPERATION_ROOTS_COVERAGE;
            }
        }
    }
    *failed_path = NULL; return SOURCE_OPERATION_ROOTS_OK;
}
