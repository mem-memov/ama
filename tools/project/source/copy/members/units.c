/** @file
 * @brief Translate actual structural unit causes through one ready main map and join exact files.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids guessing missing correspondence or resolving overlaps by order/content
 * deduplication; [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains
 * every raw unit member including opaque bytes and empty plus directories.
 */
#include "units.h"
#include <string.h>
/** Failed prefixes cannot become complete physical maps. */
static SourceCopyUnitsStatus fail(SourceFileMap *files, SourceCopyUnitsStatus status) {
    files->status = status == SOURCE_COPY_UNITS_MEMORY ? SOURCE_FILE_MAP_MEMORY : SOURCE_FILE_MAP_INVALID; return status;
}
SourceCopyUnitsStatus source_copy_map_unit_members(SourceFileMap *files, const SourceCopyClosure *copy,
    const SourceCloneMap *mains, SourceCopyContextOmissions *omissions, SourceCopyUnitsIssue *issue) {
    if (!issue) { return SOURCE_COPY_UNITS_INVALID; }
    *issue = (SourceCopyUnitsIssue){0};
    if (!files || files->ready || files->status != SOURCE_FILE_MAP_OK) { return SOURCE_COPY_UNITS_INVALID; }
    if (!copy || !copy->valid || !mains || !mains->ready || mains->capture != copy->tree->capture) {
        return fail(files, SOURCE_COPY_UNITS_INVALID);
    }
    if (omissions && (omissions->copy != copy || omissions->mains != mains || omissions->ready
        || omissions->status != SOURCE_COPY_CONTEXT_OK)) { return fail(files, SOURCE_COPY_UNITS_CONTEXT); }
    for (const AcquisitionMember *member = copy->closure.members.first; member; member = member->next) {
        for (const AcquisitionMemberCause *cause = member->causes; cause; cause = cause->next) {
            if (cause->kind != ACQUISITION_UNIT_MEMBER) { continue; }
            *issue = (SourceCopyUnitsIssue){.member = member, .cause = cause};
            const SourceCloneMapping *clone = mains->first;
            while (clone && strcmp(clone->original->logical_path, cause->requester->identity)) { clone = clone->next; }
            issue->clone = clone;
            if (!clone) {
                if (!omissions || cause->requester->role != ACQUISITION_CONTEXT) { return fail(files, SOURCE_COPY_UNITS_MAIN); }
                issue->context_status = source_copy_context_omit(omissions, cause->requester);
                if (issue->context_status != SOURCE_COPY_CONTEXT_OK) {
                    return fail(files, issue->context_status == SOURCE_COPY_CONTEXT_MEMORY ? SOURCE_COPY_UNITS_MEMORY : SOURCE_COPY_UNITS_CONTEXT);
                }
                continue;
            }
            issue->path_status = source_unit_member_path(&files->arena, source_path(clone->original->text),
                clone->path, member->path, member->directory, &issue->path);
            if (issue->path_status != SOURCE_UNIT_PATH_OK) {
                return fail(files, issue->path_status == SOURCE_UNIT_PATH_MEMORY ? SOURCE_COPY_UNITS_MEMORY : SOURCE_COPY_UNITS_PATH);
            }
            const SourceFileMapping *row = NULL;
            SourceFileMapStatus status = source_file_map_observation(files, member->path, member->content,
                member->directory, issue->path, &row);
            if (status != SOURCE_FILE_MAP_OK) { return status == SOURCE_FILE_MAP_MEMORY ? SOURCE_COPY_UNITS_MEMORY : SOURCE_COPY_UNITS_MAP; }
        }
    }
    *issue = (SourceCopyUnitsIssue){0}; return SOURCE_COPY_UNITS_OK;
}
