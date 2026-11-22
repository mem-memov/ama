/** @file
 * @brief Check actual final parent provenance for every recorded context omission.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * leaves existing destination parents unchanged. [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * supplies plus ancestry through common final discovery, not basename guessing.
 */
#include "omissions.h"
#include <string.h>
/** Exact final names are already canonical and unique in checked captures. */
static const SourceCapturedUnit *unit(const SourceCapturedUnit *units, const char *identity) {
    while (units && strcmp(units->source->logical_path, identity)) { units = units->next; }
    return units;
}
/** Original and final main queries use the same injective relation. */
static const SourceCloneMapping *clone(const SourceCloneMap *map, const char *identity, bool original) {
    const SourceCloneMapping *row = map->first;
    while (row && strcmp(original ? row->original->logical_path : row->identity, identity)) { row = row->next; }
    return row;
}
/** Compare complete captured main/dot-companion sets without entering subtrees. */
static bool same_unit(const SourceCapturedUnit *old, const SourceCapturedUnit *now) {
    const AcquisitionUnitEntry *left = old->files, *right = now->files;
    while (left && right) {
        if (strcmp(left->file.path, right->file.path) || left->directory != right->directory
            || (!left->directory && (source_length(left->file.content) != source_length(right->file.content)
                || memcmp(source_bytes(left->file.content), source_bytes(right->file.content), source_length(left->file.content))))) { return false; }
        left = left->next; right = right->next;
    }
    return !left && !right;
}
/** Both logical and physical boundaries are required, keeping nested configured
 * roots from turning an unrelated physical prefix into a module ancestor. */
static bool parent_of(const ModuleSource *parent, const ModuleSource *child) {
    size_t name_length = strlen(parent->logical_path), path_length = strlen(source_path(parent->text));
    const char *path = source_path(child->text);
    return !strncmp(child->logical_path, parent->logical_path, name_length)
        && child->logical_path[name_length] == '/'
        && !strncmp(path, source_path(parent->text), path_length)
        && path[path_length] == '+' && path[path_length + 1] == '/';
}
/** A copied parent is explicit map material; a retained parent must preserve its
 * original identity/path/whole companion set and remain outside original removal. */
static SourceCopyContextStatus provider(const SourceCopyContextOmissions *owner,
    const SourceCapturedUnit *parent, const char *const *removed, size_t count) {
    const char *identity = parent->source->logical_path;
    const SourceCloneMapping *mapped = clone(owner->mains, identity, false);
    if (mapped) {
        return !strcmp(mapped->path, source_path(parent->source->text)) ? SOURCE_COPY_CONTEXT_OK : SOURCE_COPY_CONTEXT_MISSING;
    }
    for (size_t i = 0; i < count; i++) {
        if (!strcmp(removed[i], identity)) { return SOURCE_COPY_CONTEXT_MISSING; }
    }
    const SourceCapturedUnit *old = unit(owner->copy->tree->capture->units, identity);
    if (!old || strcmp(source_path(old->source->text), source_path(parent->source->text))) { return SOURCE_COPY_CONTEXT_MISSING; }
    return same_unit(old, parent) ? SOURCE_COPY_CONTEXT_OK : SOURCE_COPY_CONTEXT_CHANGED;
}
SourceCopyContextStatus source_copy_context_omissions_check(const SourceCopyContextOmissions *owner,
    const SourceImageCapture *final, const char *const *removed, size_t count, SourceCopyContextIssue *issue) {
    if (!issue) { return SOURCE_COPY_CONTEXT_INVALID; }
    *issue = (SourceCopyContextIssue){0};
    if (!owner || !owner->ready || owner->status != SOURCE_COPY_CONTEXT_OK || !final || !final->valid
        || (count && !removed)) { return SOURCE_COPY_CONTEXT_INVALID; }
    for (size_t i = 0; i < count; i++) { if (!removed[i]) { return SOURCE_COPY_CONTEXT_INVALID; } }
    for (const SourceCopyContextOmission *omission = owner->first; omission; omission = omission->next) {
        issue->context = omission->role;
        for (const AcquisitionReason *reason = omission->role->reasons; reason; reason = reason->next) {
            if (reason->kind != ACQUISITION_TOPOLOGY || !reason->requester) { return SOURCE_COPY_CONTEXT_INVALID; }
            issue->requester = reason->requester;
            const SourceCloneMapping *mapped = clone(owner->mains, reason->requester->identity, true);
            const SourceCapturedUnit *child = mapped ? unit(final->units, mapped->identity) : NULL;
            if (!child || strcmp(source_path(child->source->text), mapped->path)) { return SOURCE_COPY_CONTEXT_MISSING; }
            for (const SourceCapturedUnit *parent = final->units; parent; parent = parent->next) {
                if (!parent_of(parent->source, child->source)) { continue; }
                issue->parent = parent;
                SourceCopyContextStatus status = provider(owner, parent, removed, count);
                if (status != SOURCE_COPY_CONTEXT_OK) { return status; }
            }
        }
    }
    *issue = (SourceCopyContextIssue){0};
    return SOURCE_COPY_CONTEXT_OK;
}
