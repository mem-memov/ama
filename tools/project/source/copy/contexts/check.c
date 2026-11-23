/** @file
 * @brief Check actual final parent provenance for every recorded context omission.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * leaves existing destination parents unchanged. [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * supplies plus ancestry through common final discovery, not basename guessing.
 */
#include "internal.h"
#include "../../capture/preservation.h"
#include <string.h>
/** A copied parent is explicit map material; a retained parent must preserve its
 * observed physical main/companions; full semantic supplier admission is separate. */
static SourceCopyContextStatus provider(const SourceCopyContextOmissions *owner,
    const SourceCapturedUnit *parent, const SourceObservedImage *observed, const char *const *removed, size_t count) {
    const char *identity = parent->source->logical_path;
    const SourceCloneMapping *mapped = source_copy_context_clone(owner->mains, identity, false);
    if (mapped) {
        return !strcmp(mapped->path, source_path(parent->source->text)) ? SOURCE_COPY_CONTEXT_OK : SOURCE_COPY_CONTEXT_MISSING;
    }
    const SourceCapturedUnit *old = source_copy_context_unit(owner->copy->tree->capture->units, identity);
    if (old && !strcmp(source_path(old->source->text), source_path(parent->source->text))) {
        for (size_t i = 0; i < count; i++) {
            if (!strcmp(removed[i], identity)) { return SOURCE_COPY_CONTEXT_MISSING; }
        }
    }
    return source_captured_unit_preserved(parent, &observed->image) ? SOURCE_COPY_CONTEXT_OK : SOURCE_COPY_CONTEXT_CHANGED;
}
SourceCopyContextStatus source_copy_context_omissions_check(const SourceCopyContextOmissions *owner,
    const SourceObservedImage *observed, const SourceImageCapture *final, const char *const *removed, size_t count, SourceCopyContextIssue *issue) {
    if (!issue) { return SOURCE_COPY_CONTEXT_INVALID; }
    *issue = (SourceCopyContextIssue){0};
    if (!owner || !owner->ready || owner->status != SOURCE_COPY_CONTEXT_OK || !final || !final->valid
        || !owner->copy || !owner->copy->valid || !owner->mains || !owner->mains->ready
        || owner->mains->capture != owner->copy->tree->capture
        || !observed || !observed->valid || !observed->sealed || observed->original != owner->copy->tree->capture
        || (count && !removed)) { return SOURCE_COPY_CONTEXT_INVALID; }
    for (size_t i = 0; i < count; i++) { if (!removed[i]) { return SOURCE_COPY_CONTEXT_INVALID; } }
    for (const SourceCopyContextOmission *omission = owner->first; omission; omission = omission->next) {
        issue->context = omission->role;
        for (const AcquisitionReason *reason = omission->role->reasons; reason; reason = reason->next) {
            if (reason->kind != ACQUISITION_TOPOLOGY || !reason->requester) { return SOURCE_COPY_CONTEXT_INVALID; }
            issue->requester = reason->requester;
            const SourceCloneMapping *mapped = source_copy_context_clone(owner->mains, reason->requester->identity, true);
            const SourceCapturedUnit *child = mapped ? source_copy_context_unit(final->units, mapped->identity) : NULL;
            if (!child || strcmp(source_path(child->source->text), mapped->path)) { return SOURCE_COPY_CONTEXT_MISSING; }
            for (const SourceCapturedUnit *parent = final->units; parent; parent = parent->next) {
                if (!source_copy_context_parent(parent->source, child->source)) { continue; }
                issue->parent = parent;
                SourceCopyContextStatus status = provider(owner, parent, observed, removed, count);
                if (status != SOURCE_COPY_CONTEXT_OK) { return status; }
            }
        }
    }
    *issue = (SourceCopyContextIssue){0};
    return SOURCE_COPY_CONTEXT_OK;
}
