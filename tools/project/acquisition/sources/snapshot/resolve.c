/** @file
 * @brief Publish one validated transport commit without changing an already known snapshot.
 * [PKG-06](../../../../../spec/project/packages.md#pkg-06) pins all reads to
 * one resolved commit and forbids a later request from changing its source.
 */
#include <string.h>
#include "../internal.h"
AcquisitionSourcesStatus acquisition_snapshot_resolve(AcquisitionSources *sources, const AcquisitionSnapshot *snapshot,
    const char *commit, AcquisitionSourcesIssue *issue) {
    if (!sources || !snapshot || !commit || !*commit || !issue) {
        return acquisition_sources_failure(issue, (SourceSpan){0}, ACQUISITION_SOURCES_INVALID, "Invalid resolved snapshot publication", NULL);
    }
    AcquisitionSnapshotRecord *record = acquisition_snapshot_record(sources, snapshot);
    if (!record) { return acquisition_sources_failure(issue, (SourceSpan){0}, ACQUISITION_SOURCES_INVALID, "Snapshot handle does not belong to this operation", NULL); }
    if (record->value.commit) {
        return !strcmp(record->value.commit, commit) ? ACQUISITION_SOURCES_OK :
            acquisition_sources_failure(issue, snapshot->origin, ACQUISITION_SOURCES_CONFLICT, "The exact snapshot request already has a different resolved commit", NULL);
    }
    char *owned = source_arena_copy(&sources->arena, commit);
    if (!owned) { return acquisition_sources_failure(issue, snapshot->origin, ACQUISITION_SOURCES_OUT_OF_MEMORY, "Cannot retain the resolved commit", NULL); }
    record->value.commit = owned;
    return ACQUISITION_SOURCES_OK;
}
