/** @file
 * @brief Normalize an explicit future destination after exact verified-prefix traversal, without filesystem creation.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits exact new destinations; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * can initialize missing version directories. [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * still rejects aliases and unverified cancelled traversal, rather than guessing
 * that a missing prefix was an existing directory.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Append only ordinary future components. A missing-prefix proposal cannot
 * justify navigation or filesystem-kind claims under
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02);
 * failed output remains private. */
static SourcePhysicalStatus tail(SourceArena *arena, const SourcePhysicalFrontier *frontier,
    const char **output, SourcePhysicalIssue *issue) {
    size_t prefix = strlen(frontier->path), length = strlen(frontier->remaining);
    if (prefix > SIZE_MAX - 2 || length > SIZE_MAX - prefix - 2) {
        return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, frontier->path, PATH_IO_ERROR);
    }
    char *buffer = source_arena_allocate(arena, prefix + length + 2);
    if (!buffer) { return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, frontier->path, PATH_IO_ERROR); }
    memcpy(buffer, frontier->path, prefix); size_t used = prefix;
    const char *cursor = frontier->remaining;
    while (*cursor) {
        while (*cursor == '/') { cursor++; }
        if (!*cursor) { break; }
        const char *end = strchr(cursor, '/'); size_t width = end ? (size_t)(end - cursor) : strlen(cursor);
        if ((width == 1 && cursor[0] == '.') || (width == 2 && cursor[0] == '.' && cursor[1] == '.')) {
            return SOURCE_PHYSICAL_MISSING;
        }
        buffer[used++] = '/'; memcpy(buffer + used, cursor, width); used += width;
        cursor += width;
    }
    buffer[used] = 0; *output = buffer; return SOURCE_PHYSICAL_OK;
}
SourcePhysicalStatus source_physical_destination(SourceArena *arena, const char *input, SourcePhysicalKind kind,
    SourcePhysicalPath *output, SourcePhysicalIssue *issue) {
    if (!output) { return source_physical_fail(issue, SOURCE_PHYSICAL_INVALID, NULL, PATH_IO_ERROR); }
    SourcePhysicalPath endpoint; SourcePhysicalFrontier frontier;
    SourcePhysicalStatus status = source_physical_walk(arena, input, kind, true, &endpoint, issue, &frontier);
    if (status == SOURCE_PHYSICAL_OK) { *output = endpoint; return status; }
    if (status != SOURCE_PHYSICAL_MISSING || !frontier.path) { return status; }
    const char *path = NULL;
    status = tail(arena, &frontier, &path, issue);
    if (status != SOURCE_PHYSICAL_OK) { return status; }
    *output = (SourcePhysicalPath){.path = path,
        .directory_required = kind == SOURCE_PHYSICAL_DIRECTORY || input[strlen(input)-1] == '/'};
    *issue = (SourcePhysicalIssue){0}; return SOURCE_PHYSICAL_OK;
}
