/**
 * @file
 * @brief Queue unique physical namespace directories without recursive C traversal.
 *
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) requires physical
 * alias rejection. Queue records/paths live in the catalog arena, preserving
 * failure diagnostics and cleanup without a separate allocation owner.
 */
#include "internal.h"

/** Register before enqueueing so two pending paths cannot revisit one directory.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) supplies the
 * device/inode uniqueness invariant across all configured ordinary roots.
 */
ModuleStatus package_queue(PackageScan *scan, const char *physical,
    const char *relative, const char *logical, size_t depth) {
    ModuleStatus status = module_remember_directory(scan->catalog, physical);
    if (status != MODULE_OK) {
        return status;
    }
    PackageDirectory *directory = source_arena_allocate(&scan->catalog->arena, sizeof(*directory));
    if (directory == NULL) {
        return module_fail(scan->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    *directory = (PackageDirectory){NULL, physical, relative, logical, depth};
    if (scan->last == NULL) {
        scan->pending = directory;
    } else {
        scan->last->next = directory;
    }
    scan->last = directory;
    return MODULE_OK;
}
