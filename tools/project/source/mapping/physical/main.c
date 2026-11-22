/** @file
 * @brief Insert physical plus markers only at exact final module-ancestor boundaries.
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05) separates
 * logical identity from storage. [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires added copies and tests to use final topology rather than original plus
 * placement. A complete proposed index precedes this pure path construction.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Exact bounded original prefix membership, with no suffix or basename search. */
static bool main_prefix(const char *const *mains, size_t count, const char *identity, size_t length) {
    for (size_t i = 0; i < count; i++) {
        if (strlen(mains[i]) == length && !memcmp(mains[i], identity, length)) { return true; }
    }
    return false;
}
SourceMainPathStatus source_main_physical_path(SourceArena *arena, const SourceVersionPlacement *version,
    const char *const *mains, size_t count, const char *identity, const char **output) {
    size_t boundary = 0;
    if (!arena || !output || !source_main_placement_valid(version, mains, count, identity, &boundary)) {
        return SOURCE_MAIN_PATH_INVALID;
    }
    size_t length = strlen(identity);
    if (!main_prefix(mains, count, identity, length)) { return SOURCE_MAIN_PATH_MISSING; }
    size_t pluses = 0;
    for (size_t i = boundary + 1; i < length; i++) {
        if (identity[i] == '/' && main_prefix(mains, count, identity, i)) { pluses++; }
    }
    size_t root_length = strlen(version->physical);
    const char *suffix = identity + boundary;
    if (version->physical[root_length - 1] == '/') { suffix++; }
    size_t suffix_length = strlen(suffix);
    if (root_length > SIZE_MAX - suffix_length || root_length + suffix_length > SIZE_MAX - pluses ||
        root_length + suffix_length + pluses == SIZE_MAX) { return SOURCE_MAIN_PATH_MEMORY; }
    char *path = source_arena_allocate(arena, root_length + suffix_length + pluses + 1);
    if (!path) { return SOURCE_MAIN_PATH_MEMORY; }
    memcpy(path, version->physical, root_length); size_t used = root_length;
    for (const char *cursor = suffix; *cursor; cursor++) {
        if (*cursor == '/' && main_prefix(mains, count, identity, (size_t)(cursor - identity))) { path[used++] = '+'; }
        path[used++] = *cursor;
    }
    path[used] = 0; *output = path;
    return SOURCE_MAIN_PATH_OK;
}
