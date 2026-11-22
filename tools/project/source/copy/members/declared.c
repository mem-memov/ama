/** @file
 * @brief Project exact declared input causes without losing shared-placement conflicts.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves non-module paths relative to their explicitly bound version;
 * configurations are environments, never opaque files to overwrite destinations.
 */
#include "declared.h"
#include <string.h>
/** Refused prefixes remain inspectable but cannot become complete maps. */
static SourceCopyDeclaredStatus fail(SourceFileMap *files, SourceCopyDeclaredStatus status) {
    if (files->status == SOURCE_FILE_MAP_OK) {
        files->status = status == SOURCE_COPY_DECLARED_MEMORY ? SOURCE_FILE_MAP_MEMORY : SOURCE_FILE_MAP_INVALID;
    }
    return status;
}
/** Selected original version configurations have their own transformation owner. */
static bool configuration(const SourceRootCapture *capture, const char *path) {
    for (const ModuleVersion *version = capture->catalog.versions; version; version = version->next) {
        if (!strcmp(source_path(version->configuration), path)) { return true; }
    }
    return false;
}
/** A version-relative fallback uses the requiring copied role, not the input's
 * basename or an unrelated version that happens to contain matching bytes. */
static SourceCopyDeclaredStatus destination(SourceFileMap *files, const SourceCopyDestinations *destinations,
    const SourceVersionPlacement *versions, size_t count, SourceCopyDeclaredIssue *issue) {
    if (!issue->cause->source) { return SOURCE_COPY_DECLARED_UNPLACED; }
    for (size_t i = 0; i < destinations->count; i++) {
        if (destinations->entries[i].role == issue->cause->requester) { issue->requester = &destinations->entries[i]; }
    }
    if (!issue->requester) { return SOURCE_COPY_DECLARED_UNPLACED; }
    const SourceVersionPlacement *version = NULL;
    for (size_t i = 0; i < count; i++) {
        if (!strcmp(versions[i].identity, issue->requester->version)) { version = &versions[i]; }
    }
    if (!version) { return SOURCE_COPY_DECLARED_VERSION; }
    const char *root = issue->cause->source->directory;
    if (!root || root[0] != '/') { return SOURCE_COPY_DECLARED_PATH; }
    size_t length = strlen(root);
    const char *path = issue->member->path;
    if (strncmp(path, root, length) || (root[length - 1] != '/' && path[length] != '/' && path[length])) {
        return SOURCE_COPY_DECLARED_PATH;
    }
    const char *suffix = path + length;
    if (*suffix == '/') { suffix++; }
    const char *separator = *suffix && version->physical[strlen(version->physical) - 1] != '/' ? "/" : "";
    issue->path = source_arena_join(&files->arena, version->physical, separator, suffix);
    return issue->path ? SOURCE_COPY_DECLARED_OK : SOURCE_COPY_DECLARED_MEMORY;
}
SourceCopyDeclaredStatus source_copy_map_declared_members(SourceFileMap *files, const SourceCopyDestinations *destinations,
    const SourceVersionPlacement *versions, size_t count, SourceCopyDeclaredIssue *issue) {
    if (!issue) { return SOURCE_COPY_DECLARED_INVALID; }
    *issue = (SourceCopyDeclaredIssue){0};
    if (!files || files->ready || files->status != SOURCE_FILE_MAP_OK) { return SOURCE_COPY_DECLARED_INVALID; }
    if (!destinations || !destinations->valid || !destinations->copy || !destinations->copy->valid) {
        return fail(files, SOURCE_COPY_DECLARED_INVALID);
    }
    if (!source_version_placements_valid(versions, count)) { return fail(files, SOURCE_COPY_DECLARED_VERSION); }
    const SourceCopyClosure *copy = destinations->copy;
    for (const AcquisitionMember *member = copy->closure.members.first; member; member = member->next) {
        if (configuration(copy->tree->capture, member->path)) { continue; }
        const SourceFileMapping *fixed = files->first;
        while (fixed && strcmp(fixed->source, member->path)) { fixed = fixed->next; }
        /* Snapshot the structural placement before inserting this member's first
         * cause: a fallback inserted here must not hide a later cause's conflict. */
        for (const AcquisitionMemberCause *cause = member->causes; cause; cause = cause->next) {
            if (cause->kind != ACQUISITION_DECLARED_INPUT) { continue; }
            *issue = (SourceCopyDeclaredIssue){.member = member, .cause = cause, .path = fixed ? fixed->destination : NULL};
            SourceCopyDeclaredStatus status = fixed ? SOURCE_COPY_DECLARED_OK : destination(files, destinations, versions, count, issue);
            if (status != SOURCE_COPY_DECLARED_OK) { return fail(files, status); }
            const SourceFileMapping *row = NULL;
            SourceFileMapStatus added = source_file_map_observation(files, member->path, member->content,
                member->directory, issue->path, &row);
            if (added != SOURCE_FILE_MAP_OK) {
                return fail(files, added == SOURCE_FILE_MAP_MEMORY ? SOURCE_COPY_DECLARED_MEMORY : SOURCE_COPY_DECLARED_MAP);
            }
        }
    }
    *issue = (SourceCopyDeclaredIssue){0};
    return SOURCE_COPY_DECLARED_OK;
}
