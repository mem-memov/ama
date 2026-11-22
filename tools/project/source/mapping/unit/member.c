/** @file
 * @brief Recognize disjoint physical unit forms before owning their translated key.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) defines main/flat
 * companions/full plus tree; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * translates only the selected main stem and preserves every retained suffix byte.
 */
#include "member.h"
#include <string.h>
/** Main keys are already normalized; reject root/directory-shaped interfaces. */
static bool main_key(const char *path) {
    return path && path[0] == '/' && path[1] && path[strlen(path) - 1] != '/';
}
/** Classify without allocations: no basename/prefix approximation admits peers. */
static const char *member_suffix(const char *path, bool directory, const char *main) {
    size_t length = strlen(main);
    if (strncmp(path, main, length)) { return NULL; }
    const char *suffix = path + length;
    if (!*suffix) { return directory ? NULL : suffix; }
    if (*suffix == '.' && suffix[1] && !strchr(suffix, '/')) { return directory ? NULL : suffix; }
    if (*suffix == '+' && (suffix[1] == '/' || !suffix[1])) {
        return !suffix[1] && !directory ? NULL : suffix;
    }
    return NULL;
}
SourceUnitPathStatus source_unit_member_path(SourceArena *arena, const char *original,
    const char *destination, const char *member, bool directory, const char **output) {
    if (!arena || !output || !main_key(original) || !main_key(destination) || !member || member[0] != '/') {
        return SOURCE_UNIT_PATH_INVALID;
    }
    const char *suffix = member_suffix(member, directory, original);
    if (!suffix) { return SOURCE_UNIT_PATH_OUTSIDE; }
    const char *path = source_arena_join(arena, destination, "", suffix);
    if (!path) { return SOURCE_UNIT_PATH_MEMORY; }
    *output = path; return SOURCE_UNIT_PATH_OK;
}
