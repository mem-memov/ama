/** @file
 * @brief Select the unique deepest logical associated-test mapping without guessing.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * uses deepest containing associations; exact slash boundaries distinguish
 * siblings and namespace directories from equally named module files.
 */
#include "tests.h"
#include "source/path/logical.h"
#include <string.h>
/** Canonical test namespace components have the shape of a complete test name. */
static bool test_name(const char *identity) {
    ModulePathShape shape;
    return identity && module_path_describe((const unsigned char *)identity,
        strlen(identity), &shape) && shape.extent == MODULE_PATH_MODULE &&
        shape.area == MODULE_PATH_TESTS;
}
SourceTestMapStatus source_associated_test_identity(SourceArena *arena,
    const SourceTestDirectoryMap *pairs, size_t count, const char *identity,
    const char **output) {
    if (!arena || !output || (count && !pairs) || !test_name(identity)) {
        return SOURCE_TEST_MAP_INVALID;
    }
    for (size_t i = 0; i < count; i++) {
        if (!test_name(pairs[i].original) || !test_name(pairs[i].destination)) {
            return SOURCE_TEST_MAP_INVALID;
        }
    }
    const SourceTestDirectoryMap *deepest = NULL;
    size_t depth = 0, length = strlen(identity);
    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(pairs[i].original, pairs[j].original) &&
                strcmp(pairs[i].destination, pairs[j].destination)) {
                return SOURCE_TEST_MAP_CONFLICT;
            }
        }
        size_t prefix = strlen(pairs[i].original);
        if (prefix > depth && length > prefix && identity[prefix] == '/' &&
            !memcmp(identity, pairs[i].original, prefix)) {
            deepest = &pairs[i]; depth = prefix;
        }
    }
    if (!deepest) { return SOURCE_TEST_MAP_OUTSIDE; }
    const char *mapped = source_arena_join(arena, deepest->destination, "/", identity + depth + 1);
    if (!mapped) { return SOURCE_TEST_MAP_MEMORY; }
    *output = mapped;
    return SOURCE_TEST_MAP_OK;
}
