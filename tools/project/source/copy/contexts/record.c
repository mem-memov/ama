/** @file
 * @brief Account for every unmapped structural unit before freezing omission obligations.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes structural context from active units; no active main may be lost
 * merely because another parent supplies a destination directory.
 */
#include "omissions.h"
#include <string.h>
/** Exact immutable owner identity precedes every borrowed role dereference. */
static bool valid(const SourceCopyContextOmissions *owner) {
    return owner->copy && owner->copy->valid && owner->mains && owner->mains->ready
        && owner->mains->capture == owner->copy->tree->capture;
}
/** All ordinary/standard active and retained context copies share one map. */
static bool mapped(const SourceCopyContextOmissions *owner, const AcquisitionRoleRecord *role) {
    for (const SourceCloneMapping *row = owner->mains->first; row; row = row->next) {
        if (!strcmp(row->original->logical_path, role->identity)) { return true; }
    }
    return false;
}
/** Closure member causes, rather than a caller's role-shaped stack object, authorize omission. */
static bool context_member(const SourceCopyContextOmissions *owner, const AcquisitionRoleRecord *role) {
    for (const AcquisitionMember *member = owner->copy->closure.members.first; member; member = member->next) {
        for (const AcquisitionMemberCause *cause = member->causes; cause; cause = cause->next) {
            if (cause->kind == ACQUISITION_UNIT_MEMBER && cause->requester == role) { return true; }
        }
    }
    return false;
}
/** Unique obligations make repeated context companions idempotent. */
static bool recorded(const SourceCopyContextOmissions *owner, const AcquisitionRoleRecord *role) {
    for (const SourceCopyContextOmission *row = owner->first; row; row = row->next) {
        if (row->role == role) { return true; }
    }
    return false;
}
SourceCopyContextStatus source_copy_context_omit(SourceCopyContextOmissions *owner, const AcquisitionRoleRecord *role) {
    if (!owner || owner->ready) { return SOURCE_COPY_CONTEXT_INVALID; }
    if (owner->status != SOURCE_COPY_CONTEXT_OK) { return owner->status; }
    if (!valid(owner) || !role || !context_member(owner, role)
        || role->role != ACQUISITION_CONTEXT || mapped(owner, role)) {
        return owner->status = SOURCE_COPY_CONTEXT_INVALID;
    }
    if (recorded(owner, role)) { return SOURCE_COPY_CONTEXT_OK; }
    SourceCopyContextOmission *row = source_arena_allocate(&owner->arena, sizeof(*row));
    if (!row) { return owner->status = SOURCE_COPY_CONTEXT_MEMORY; }
    *row = (SourceCopyContextOmission){owner->first, role};
    owner->first = row;
    return SOURCE_COPY_CONTEXT_OK;
}
SourceCopyContextStatus source_copy_context_omissions_seal(SourceCopyContextOmissions *owner) {
    if (!owner || owner->ready) { return SOURCE_COPY_CONTEXT_INVALID; }
    if (owner->status != SOURCE_COPY_CONTEXT_OK) { return owner->status; }
    if (!valid(owner)) { return owner->status = SOURCE_COPY_CONTEXT_INVALID; }
    for (const AcquisitionMember *member = owner->copy->closure.members.first; member; member = member->next) {
        for (const AcquisitionMemberCause *cause = member->causes; cause; cause = cause->next) {
            if (cause->kind != ACQUISITION_UNIT_MEMBER || mapped(owner, cause->requester)) { continue; }
            if (cause->requester->role != ACQUISITION_CONTEXT || !recorded(owner, cause->requester)) {
                return owner->status = SOURCE_COPY_CONTEXT_MISSING;
            }
        }
    }
    owner->ready = true;
    return SOURCE_COPY_CONTEXT_OK;
}
