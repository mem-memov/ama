/** @file
 * @brief Verify physical components before lexical reduction and publish only complete endpoint observations.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) rejects alias traversal;
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes source shape from missing final destinations;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed scratch.
 */
#include <string.h>
#include "internal.h"
/** Current is an already verified absolute directory; only then is its lexical
 * parent valid under [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 */
static const char *parent(SourceArena *arena, const char *current) {
    char *copy = source_arena_copy(arena, current);
    if (!copy) { return NULL; }
    char *slash = strrchr(copy, '/');
    if (slash == copy) { slash[1] = 0; } else { *slash = 0; }
    return copy;
}
/** Exact probe outcomes retain missing versus technical/alias failure under
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 */
static SourcePhysicalStatus failure(SourcePhysicalIssue *issue, const char *path, PathProbe probe) {
    SourcePhysicalStatus status = probe == PATH_MISSING ? SOURCE_PHYSICAL_MISSING
        : probe == PATH_UNSUPPORTED || probe == PATH_FOUND ? SOURCE_PHYSICAL_UNSUPPORTED : SOURCE_PHYSICAL_IO_ERROR;
    return source_physical_fail(issue, status, path, probe);
}
/** Final assignment alone publishes an accepted observation under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
static SourcePhysicalStatus publish(SourcePhysicalPath *output, SourcePhysicalIssue *issue,
    const char *path, const struct stat *metadata, bool directory_required) {
    *output = (SourcePhysicalPath){.path = path, .present = metadata != NULL, .directory_required = directory_required};
    if (metadata) { output->metadata = *metadata; }
    *issue = (SourcePhysicalIssue){0}; return SOURCE_PHYSICAL_OK;
}
SourcePhysicalStatus source_physical_resolve(SourceArena *arena, const char *input, SourcePhysicalKind kind,
    bool missing_final, SourcePhysicalPath *output, SourcePhysicalIssue *issue) {
    if (!arena || !input || !*input || !output || !issue
        || (kind != SOURCE_PHYSICAL_DIRECTORY && kind != SOURCE_PHYSICAL_ENDPOINT)) {
        return source_physical_fail(issue, SOURCE_PHYSICAL_INVALID, NULL, PATH_IO_ERROR);
    }
    bool directory_required = kind == SOURCE_PHYSICAL_DIRECTORY || input[strlen(input) - 1] == '/';
    char *copy = source_arena_copy(arena, input);
    if (!copy) { return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, NULL, PATH_IO_ERROR); }
    const char *current = "/";
    if (*copy != '/') {
        SourcePhysicalStatus status = source_physical_cwd(arena, &current, issue);
        if (status != SOURCE_PHYSICAL_OK) { return status; }
    }
    char *cursor = copy;
    while (*cursor) {
        while (*cursor == '/') { cursor++; }
        if (!*cursor) { break; }
        char *component = cursor;
        while (*cursor && *cursor != '/') { cursor++; }
        if (*cursor) { *cursor++ = 0; }
        char *remaining = cursor;
        while (*remaining == '/') { remaining++; }
        if (!strcmp(component, ".")) { continue; }
        if (!strcmp(component, "..")) {
            const char *above = parent(arena, current);
            if (!above) { return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, current, PATH_IO_ERROR); }
            current = above; continue;
        }
        const char *full = source_arena_join(arena, current, !strcmp(current, "/") ? "" : "/", component);
        if (!full) { return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, current, PATH_IO_ERROR); }
        struct stat metadata;
        PathProbe probe = path_exact_child(current, component, full, &metadata);
        bool final = !*remaining;
        if (probe == PATH_MISSING && missing_final && final) {
            return publish(output, issue, full, NULL, directory_required);
        }
        if (probe != PATH_FOUND) { return failure(issue, full, probe); }
        if ((!S_ISDIR(metadata.st_mode) && !S_ISREG(metadata.st_mode))
            || ((!final || directory_required) && !S_ISDIR(metadata.st_mode))) { return failure(issue, full, PATH_UNSUPPORTED); }
        if (final) { return publish(output, issue, full, &metadata, directory_required); }
        current = full;
    }
    struct stat metadata;
    if (lstat(current, &metadata)) { return source_physical_fail(issue, SOURCE_PHYSICAL_IO_ERROR, current, PATH_IO_ERROR); }
    if (!S_ISDIR(metadata.st_mode)) { return failure(issue, current, PATH_UNSUPPORTED); }
    return publish(output, issue, current, &metadata, true);
}
