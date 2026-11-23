/** @file
 * @brief Preserve retained original units and prove every cloned main and companion placement.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * applies to drafts and edge-free definitions as well as active dependencies.
 */
#include <string.h>
#include "internal.h"
/** Names are exact canonical identities; removed drafts need no active export. */
static bool removed_name(const char *name, const char *const *removed, size_t count) {
    for (size_t i = 0; i < count; i++) { if (!strcmp(name, removed[i])) { return true; } }
    return false;
}
/** Common capture provides one actual unit per canonical identity. */
static const SourceCapturedUnit *find(const SourceCapturedUnit *units, const char *name) {
    while (units && strcmp(units->source->logical_path, name)) { units = units->next; }
    return units;
}
/** Exact paths and bytes preserve opaque/native/local-INI companions too. */
static bool same(const SourceCapturedUnit *left, const SourceCapturedUnit *right) {
    const AcquisitionUnitEntry *a = left->files, *b = right->files;
    if (strcmp(source_path(left->source->text), source_path(right->source->text))) { return false; }
    while (a && b) {
        if (a->directory != b->directory || strcmp(a->file.path, b->file.path)
            || (!a->directory && (source_length(a->file.content) != source_length(b->file.content)
                || memcmp(source_bytes(a->file.content), source_bytes(b->file.content), source_length(a->file.content))))) { return false; }
        a = a->next; b = b->next;
    }
    return !a && !b;
}
/** Physical companion membership is separate from semantic graph membership. */
static bool mapped_file(const SourceFileMap *files, const char *path) {
    for (const SourceFileMapping *row = files->first; row; row = row->next) {
        if (!strcmp(row->destination, path)) { return true; }
    }
    return false;
}
SourceCopyProofStatus source_copy_proof_units(const SourceCloneMap *map, const SourceFileMap *files,
    const SourceImageCapture *final, const char *const *removed, size_t count,
    SourceRegistryRetirement retired, SourceCopyProofIssue *issue) {
    for (const SourceCloneMapping *row = map->first; row; row = row->next) {
        issue->unit = find(final->units, row->identity); issue->path = row->path;
        if (!issue->unit || strcmp(source_path(issue->unit->source->text), row->path)) { return SOURCE_COPY_PROOF_PLACEMENT; }
        for (const AcquisitionUnitEntry *file = issue->unit->files; file; file = file->next) {
            issue->path = file->file.path;
            if (!mapped_file(files, issue->path)) { return SOURCE_COPY_PROOF_PLACEMENT; }
        }
    }
    for (const SourceCapturedUnit *old = map->capture->units; old; old = old->next) {
        if (removed_name(old->source->logical_path, removed, count)) { continue; }
        if (removed_name(old->source->logical_path, retired.names, retired.count)) {
            issue->unit = old; issue->path = source_path(old->source->text);
            if (!source_copy_proof_preserved_unit(old, final->image)) { return SOURCE_COPY_PROOF_ORIGINAL; }
            continue;
        }
        issue->unit = find(final->units, old->source->logical_path); issue->path = source_path(old->source->text);
        if (!issue->unit || !same(old, issue->unit)) { return SOURCE_COPY_PROOF_ORIGINAL; }
    }
    for (const SourceCapturedUnit *now = final->units; now; now = now->next) {
        const char *name = now->source->logical_path;
        if (!removed_name(name, retired.names, retired.count) && find(map->capture->units, name)) { continue; }
        const SourceCloneMapping *row = map->first;
        while (row && strcmp(row->identity, name)) { row = row->next; }
        if (!row) { issue->unit = now; issue->path = source_path(now->source->text); return SOURCE_COPY_PROOF_PLACEMENT; }
    }
    return SOURCE_COPY_PROOF_OK;
}
