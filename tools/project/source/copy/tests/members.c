/** @file
 * @brief Join each raw test member through the deepest complete source association.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * makes nested ownership independent of discovery order and rejects overlapping
 * unit/test projections that disagree instead of silently duplicating originals.
 */
#include "internal.h"
#include <string.h>
/** Failed prefixes must never certify complete physical material. */
static SourceCopyTestsStatus fail(SourceFileMap *files, SourceCopyTestsStatus status) {
    if (files->status == SOURCE_FILE_MAP_OK) {
        files->status = status == SOURCE_COPY_TESTS_MEMORY ? SOURCE_FILE_MAP_MEMORY : SOURCE_FILE_MAP_INVALID;
    }
    return status;
}
/** Exact directory boundaries admit the root directory and strict descendants. */
static bool contains(const char *directory, const char *path) {
    size_t length = strlen(directory);
    return !strncmp(directory, path, length) && (!path[length] || path[length] == '/');
}
SourceCopyTestsStatus source_copy_map_test_members(SourceFileMap *files, const SourceCopyClosure *copy,
    const SourceCloneMap *mains, const SourceVersionPlacement *versions, size_t count, SourceCopyTestsIssue *issue) {
    if (!issue) { return SOURCE_COPY_TESTS_INVALID; }
    *issue = (SourceCopyTestsIssue){0};
    if (!files || files->ready || files->status != SOURCE_FILE_MAP_OK) { return SOURCE_COPY_TESTS_INVALID; }
    if (!copy || !copy->valid || !mains || !mains->ready || mains->capture != copy->tree->capture) {
        return fail(files, SOURCE_COPY_TESTS_INVALID);
    }
    const SourceCopyTestAssociation *pairs = NULL;
    SourceCopyTestsStatus status = source_copy_test_associations(files, copy, mains, versions, count, &pairs, issue);
    if (status != SOURCE_COPY_TESTS_OK) { return fail(files, status); }
    for (const AcquisitionMember *member = copy->closure.members.first; member; member = member->next) {
        const AcquisitionMemberCause *cause = member->causes;
        while (cause && cause->kind != ACQUISITION_TEST_MEMBER) { cause = cause->next; }
        if (!cause) { continue; }
        *issue = (SourceCopyTestsIssue){.member = member, .cause = cause};
        const SourceCopyTestAssociation *deepest = NULL;
        for (const SourceCopyTestAssociation *pair = pairs; pair; pair = pair->next) {
            if (contains(pair->original, member->path)
                && (!deepest || strlen(pair->original) > strlen(deepest->original))) { deepest = pair; }
        }
        if (!deepest) { return fail(files, SOURCE_COPY_TESTS_PATH); }
        issue->clone = deepest->clone;
        const char *suffix = member->path + strlen(deepest->original);
        if (!*suffix && !member->directory) { return fail(files, SOURCE_COPY_TESTS_PATH); }
        issue->path = source_arena_join(&files->arena, deepest->destination, "", suffix);
        if (!issue->path) { return fail(files, SOURCE_COPY_TESTS_MEMORY); }
        const SourceFileMapping *row = NULL;
        SourceFileMapStatus added = source_file_map_observation(files, member->path, member->content,
            member->directory, issue->path, &row);
        if (added != SOURCE_FILE_MAP_OK) {
            return fail(files, added == SOURCE_FILE_MAP_MEMORY ? SOURCE_COPY_TESTS_MEMORY : SOURCE_COPY_TESTS_MAP);
        }
    }
    *issue = (SourceCopyTestsIssue){0};
    return SOURCE_COPY_TESTS_OK;
}
