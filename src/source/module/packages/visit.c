/**
 * @file
 * @brief Distinguish namespace grouping directories from complete package-version boundaries.
 *
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) supplies vendor/package
 * depth and exact version spelling. [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * discards an ignored external version before reading its contents.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "source/path/version.h"
#include "source/unicode/unicode.h"

/** Preserve bounded name validation before allocating exact physical/logical paths.
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) rejects invalid
 * UTF-8; version punctuation belongs only to the separate PKG-01 production.
 */
ModuleStatus package_visit(PackageScan *scan, const PackageDirectory *directory,
    const char *name) {
    size_t length = strlen(name);
    size_t error;
    const char *physical = source_arena_join(&scan->catalog->arena, directory->physical, "/", name);
    if (physical == NULL) {
        return module_fail(scan->catalog, MODULE_OUT_OF_MEMORY, directory->physical);
    }
    if (!utf8_validate((const unsigned char *)name, length, &error)) {
        return module_fail(scan->catalog, MODULE_UNSUPPORTED_PATH, physical);
    }
    bool version = module_version_component((const unsigned char *)name, length);
    if (!version && !unicode_name((const unsigned char *)name, length, &error)) {
        return MODULE_OK;
    }
    const char *relative = source_arena_join(&scan->catalog->arena, directory->relative, "/", name);
    const char *logical = source_arena_join(&scan->catalog->arena, directory->logical, "/", name);
    if (relative == NULL || logical == NULL) {
        return module_fail(scan->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    if (version && directory->depth >= 2 && package_ignored(scan, logical)) {
        return MODULE_OK;
    }
    struct stat metadata;
    ModuleStatus status = scan->reader->child(scan->reader->context, scan->catalog, directory->physical, name, physical, &metadata);
    if (status != MODULE_OK || !S_ISDIR(metadata.st_mode)) {
        return status;
    }
    if (version) {
        return directory->depth < 2
            ? module_fail(scan->catalog, MODULE_INVALID_PATH, physical)
            : package_register(scan, physical, relative, logical);
    }
    if (directory->depth == SIZE_MAX) {
        return module_fail(scan->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    return package_queue(scan, physical, relative, logical, directory->depth + 1, &metadata);
}
