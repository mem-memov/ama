/** @file
 * @brief Derive mirrored logical test namespaces without physical source handles.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) associates tests with
 * complete logical src suffixes, independent of physical plus storage. Sharing
 * this operation keeps original delivery and final placement planning identical.
 */
#include "association.h"
#include "source/path/logical.h"
#include <string.h>
#include <stdint.h>
SourceStatus acquisition_test_namespace(SourceArena *arena, const char *identity,
    const char **output) {
    ModulePathShape shape;
    if (!arena || !identity || !output ||
        !module_path_describe((const unsigned char *)identity, strlen(identity), &shape) ||
        shape.extent != MODULE_PATH_MODULE || shape.area != MODULE_PATH_SRC) {
        return SOURCE_INVALID_ARGUMENT;
    }
    size_t length = strlen(identity);
    if (length > SIZE_MAX - 3) { return SOURCE_OUT_OF_MEMORY; }
    char *logical = source_arena_allocate(arena, length + 3);
    if (!logical) { return SOURCE_OUT_OF_MEMORY; }
    memcpy(logical, identity, shape.version_length);
    memcpy(logical + shape.version_length, "/tests/", sizeof("/tests/") - 1);
    const char *suffix = identity + shape.version_length + sizeof("/src/") - 1;
    memcpy(logical + shape.version_length + sizeof("/tests/") - 1,
        suffix, strlen(suffix) + 1);
    *output = logical;
    return SOURCE_OK;
}
