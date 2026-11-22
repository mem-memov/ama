/** @file
 * @brief Project prepared physical layout into canonical candidate identities without prefix renaming.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * derives names from placements; [PKG-01](../../../../spec/project/packages.md#pkg-01)
 * supplies version/area layout; [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * strips only module-directory storage markers. Shared validators decide name legality.
 */
#include <stdint.h>
#include <string.h>
#include "identity.h"
#include "source/path/logical.h"
#include "source/path/version.h"
/** Require a slash-boundary physical descendant, preserving an explicit root prefix. */
static const char *relative(const char *path, const char *root) {
    size_t length = strlen(root);
    if (strncmp(path, root, length)) { return NULL; }
    const char *result = path + length;
    if (!*result) { return NULL; }
    if (root[length - 1] != '/' && *result != '/') { return NULL; }
    return *result == '/' ? result + 1 : result;
}
/** Find only an exact ordinary version/area boundary; package components retain raw spelling. */
static const char *module_area(const char *body) {
    const char *cursor = body;
    while (*cursor) {
        const char *slash = strchr(cursor, '/');
        if (!slash) { return NULL; }
        if (module_version_component((const unsigned char *)cursor, (size_t)(slash - cursor))) {
            const char *area = slash + 1, *end = strchr(area, '/');
            if (!end || !end[1]) { return NULL; }
            size_t length = (size_t)(end - area);
            return (length == 3 && !memcmp(area, "src", 3)) || (length == 5 && !memcmp(area, "tests", 5)) ? end + 1 : NULL;
        }
        cursor = slash + 1;
    }
    return NULL;
}
/** Build one structural spelling, then delegate all Unicode/name/version validation. */
static SourceIdentityStatus candidate(SourceArena *arena, const ModuleRoot *root,
    const char *path, const char **output) {
    const char *body = path, *module;
    size_t leading = root->kind == MODULE_ROOT_STANDARD ? 2 : 1;
    if (root->kind == MODULE_ROOT_PROJECT) {
        if (!strncmp(body, "packages/", 9)) { body += 9; }
        else if (!strncmp(body, "dependencies/", 13)) { body += 13; }
        else { return SOURCE_IDENTITY_NOT_MODULE; }
        module = module_area(body);
        if (!module) { return SOURCE_IDENTITY_NOT_MODULE; }
    } else { module = body; }
    size_t length = strlen(body);
    if (length > SIZE_MAX - leading - 1) { return SOURCE_IDENTITY_MEMORY; }
    char *identity = source_arena_allocate(arena, length + leading + 1);
    if (!identity) { return SOURCE_IDENTITY_MEMORY; }
    memset(identity, '/', leading);
    size_t used = leading, preamble = (size_t)(module - body);
    memcpy(identity + used, body, preamble); used += preamble;
    const char *cursor = module;
    while (*cursor) {
        const char *slash = strchr(cursor, '/');
        size_t width = slash ? (size_t)(slash - cursor) : strlen(cursor);
        if (slash && width && cursor[width - 1] == '+') { width--; }
        memcpy(identity + used, cursor, width); used += width;
        if (!slash) { break; }
        identity[used++] = '/'; cursor = slash + 1;
    }
    identity[used] = 0;
    if (!module_path_canonical((const unsigned char *)identity, used)) { return SOURCE_IDENTITY_NOT_MODULE; }
    *output = identity;
    return SOURCE_IDENTITY_OK;
}
SourceIdentityStatus source_placement_identity(SourceArena *arena, const ModuleRoot *roots, size_t count,
    const char *path, SourcePlacedIdentity *output) {
    if (!arena || (count && !roots) || !path || path[0] != '/' || !output) { return SOURCE_IDENTITY_INVALID; }
    bool covered = false, found = false;
    SourcePlacedIdentity result = {0};
    for (size_t index = 0; index < count; index++) {
        const ModuleRoot *root = &roots[index];
        if (!root->path || root->path[0] != '/' || (root->excluded_count && !root->excluded)
            || (root->kind != MODULE_ROOT_STANDARD && root->kind != MODULE_ROOT_PROJECT)) { return SOURCE_IDENTITY_INVALID; }
        for (size_t item = 0; item < root->excluded_count; item++) { if (!root->excluded[item]) { return SOURCE_IDENTITY_INVALID; } }
        const char *part = relative(path, root->path);
        if (!part) { continue; }
        covered = true;
        if (module_path_excluded(root, part)) { continue; }
        const char *identity = NULL;
        SourceIdentityStatus status = candidate(arena, root, part, &identity);
        if (status == SOURCE_IDENTITY_MEMORY) { return status; }
        if (status != SOURCE_IDENTITY_OK) { continue; }
        if (found) { return SOURCE_IDENTITY_AMBIGUOUS; }
        result = (SourcePlacedIdentity){identity, index}; found = true;
    }
    if (!found) { return covered ? SOURCE_IDENTITY_NOT_MODULE : SOURCE_IDENTITY_OUTSIDE; }
    *output = result;
    return SOURCE_IDENTITY_OK;
}
