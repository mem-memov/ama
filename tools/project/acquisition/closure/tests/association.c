/** @file
 * @brief Map a complete source suffix to tests using its authoritative version.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) associates nested logical
 * modules with ordinary mirrored directories; [PKG-02](../../../../../spec/project/packages.md#pkg-02)
 * forbids borrowing tests from an ignored package copy.
 */
#include "association.h"
#include "source/path/logical.h"
#include <string.h>
SourceStatus acquisition_test_directory(SourceArena *arena, const ModuleSource *source,
    AcquisitionTestDirectory *output) {
    if (!arena || !source || !source->logical_path || !source->version || !output ||
        !source->version->logical_path || !source->version->physical_path ||
        !*source->version->physical_path) { return SOURCE_INVALID_ARGUMENT; }
    ModulePathShape shape;
    size_t length = strlen(source->logical_path);
    if (!module_path_describe((const unsigned char *)source->logical_path, length, &shape) ||
        shape.extent != MODULE_PATH_MODULE || shape.area != MODULE_PATH_SRC ||
        strlen(source->version->logical_path) != shape.version_length ||
        memcmp(source->version->logical_path, source->logical_path, shape.version_length)) {
        return SOURCE_INVALID_ARGUMENT;
    }
    /* The canonical shape proves the exact '/src/' boundary; the remaining
     * suffix is already logical, including nested module components. */
    const char *suffix = source->logical_path + shape.version_length + sizeof("/src/") - 1;
    const char *logical = source_arena_join(arena, source->version->logical_path, "/tests/", suffix);
    const char *parent = source->version->physical_path;
    const char *physical = source_arena_join(arena, parent,
        parent[strlen(parent) - 1] == '/' ? "tests/" : "/tests/", suffix);
    if (!logical || !physical) { return SOURCE_OUT_OF_MEMORY; }
    *output = (AcquisitionTestDirectory){logical, physical};
    return SOURCE_OK;
}
