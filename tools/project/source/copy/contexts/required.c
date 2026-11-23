/** @file
 * @brief Refuse unnecessary context clones and preserve existing destination parent mains.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes required new structural material from original analysis context
 * and matches necessary parent clones by nearest-first module-ancestor position.
 */
#include "internal.h"
#include <string.h>
/** Raw selected units remain explicit copy material even if they are drafts. */
static bool explicit_unit(const SourceTreeSelection *tree, const ModuleSource *source) {
    for (size_t i = 0; i < tree->count; i++) { if (tree->units[i]->source == source) { return true; } }
    return false;
}
/** Find the exact closure role; active or absent roles grant no context-only
 * copying permission and remain subject to the independent material proof. */
static const AcquisitionRoleRecord *role(const SourceCopyClosure *copy, const char *identity) {
    const AcquisitionRoleRecord *row = copy->closure.roles.records;
    while (row && strcmp(row->identity, identity)) { row = row->next; }
    return row;
}
/** Count strict intervening module parents, not path components. Actual captured
 * topology supplies a finite chain; retained parents count exactly like clones.
 * TOOL-01 matches positions, so equal counts identify the corresponding ancestor.
 * No allocation, mutation or filesystem query is needed. */
static size_t intervening(const SourceCapturedUnit *units, const ModuleSource *parent, const ModuleSource *child) {
    size_t count = 0;
    for (; units; units = units->next) {
        if (source_copy_context_parent(parent, units->source)
            && source_copy_context_parent(units->source, child)) { count++; }
    }
    return count;
}
/** One actual original topology cause must still require this final plus parent.
 * Strict final ancestry makes chains of mapped context demands acyclic. */
static bool demanded(const SourceCopyContextOmissions *owner, const SourceImageCapture *final,
    const SourceCapturedUnit *parent, const AcquisitionRoleRecord *context, SourceCopyContextIssue *issue) {
    for (const AcquisitionReason *reason = context->reasons; reason; reason = reason->next) {
        if (reason->kind != ACQUISITION_TOPOLOGY || !reason->requester) { continue; }
        const SourceCloneMapping *mapped = source_copy_context_clone(owner->mains, reason->requester->identity, true);
        const SourceCapturedUnit *child = mapped ? source_copy_context_unit(final->units, mapped->identity) : NULL;
        const SourceCapturedUnit *original_child = source_copy_context_unit(owner->mains->capture->units, reason->requester->identity);
        const SourceCapturedUnit *original_parent = source_copy_context_unit(owner->mains->capture->units, context->identity);
        if (child && original_child && original_parent && !strcmp(source_path(child->source->text), mapped->path)
            && source_copy_context_parent(original_parent->source, original_child->source)
            && source_copy_context_parent(parent->source, child->source)
            && intervening(owner->mains->capture->units, original_parent->source, original_child->source)
                == intervening(final->units, parent->source, child->source)) {
            issue->requester = reason->requester; return true;
        }
    }
    return false;
}
SourceCopyContextStatus source_copy_context_copies_check(const SourceCopyContextOmissions *owner,
    const SourceObservedImage *observed, const SourceImageCapture *final, SourceCopyContextIssue *issue) {
    if (!issue) { return SOURCE_COPY_CONTEXT_INVALID; } *issue = (SourceCopyContextIssue){0};
    if (!owner || !owner->ready || owner->status != SOURCE_COPY_CONTEXT_OK || !owner->copy || !owner->copy->valid
        || !owner->mains || !owner->mains->ready || owner->mains->capture != owner->copy->tree->capture
        || !observed || !observed->valid || !observed->sealed || observed->original != owner->mains->capture
        || !final || !final->valid) { return SOURCE_COPY_CONTEXT_INVALID; }
    for (const SourceCloneMapping *row = owner->mains->first; row; row = row->next) {
        const AcquisitionRoleRecord *context = role(owner->copy, row->original->logical_path);
        if (!context || context->role != ACQUISITION_CONTEXT || explicit_unit(owner->copy->tree, row->original)) { continue; }
        issue->context = context; issue->parent = source_copy_context_unit(final->units, row->identity);
        if (!issue->parent || strcmp(source_path(issue->parent->source->text), row->path)) { return SOURCE_COPY_CONTEXT_MISSING; }
        if (!demanded(owner, final, issue->parent, context, issue)) { return SOURCE_COPY_CONTEXT_UNNECESSARY; }
        if (source_image_path_absent(&observed->image, row->path)) { continue; }
        for (size_t i = 0; i < observed->image.count; i++) {
            const SourceCatalogImageEntry *entry = &observed->image.entries[i];
            if (!strcmp(entry->path, row->path) && entry->metadata_captured) { return SOURCE_COPY_CONTEXT_UNNECESSARY; }
        }
        return SOURCE_COPY_CONTEXT_UNOBSERVED;
    }
    *issue = (SourceCopyContextIssue){0}; return SOURCE_COPY_CONTEXT_OK;
}
