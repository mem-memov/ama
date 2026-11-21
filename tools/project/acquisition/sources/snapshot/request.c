/** @file
 * @brief Intern exact transport requests before any external resolution is scheduled.
 * [PKG-06](../../../../../spec/project/packages.md#pkg-06) resolves each
 * identical URL/optional-revision key once within an operation.
 */
#include <stdint.h>
#include "../internal.h"
AcquisitionSourcesStatus acquisition_snapshot_request(AcquisitionSources *sources, const AcquisitionRoute *route,
    const AcquisitionSnapshot **output, bool *created, AcquisitionSourcesIssue *issue) {
    const ConfigurationRepository *mapping = route->mapping;
    for (AcquisitionSnapshotRecord *record = sources->snapshots; record; record = record->next) {
        if (acquisition_source_text_equal(record->value.url, mapping->url) &&
            acquisition_source_text_equal(record->value.revision, mapping->revision)) {
            *output = &record->value; *created = false; return ACQUISITION_SOURCES_OK;
        }
    }
    if (sources->snapshot_count == SIZE_MAX) {
        return acquisition_sources_failure(issue, mapping->header, ACQUISITION_SOURCES_LIMIT, "Snapshot request count exceeds addressable storage", NULL);
    }
    AcquisitionSnapshotRecord *record = source_arena_allocate(&sources->arena, sizeof(*record));
    if (!record) { goto no_memory; }
    char *url = acquisition_source_text(sources, mapping->url);
    if (!url) { goto no_memory; }
    char *revision = mapping->revision.source ? acquisition_source_text(sources, mapping->revision) : NULL;
    if (mapping->revision.source && !revision) { goto no_memory; }
    SourceSpan origin = source_span_valid(route->request->expression) ? route->request->expression : mapping->header;
    *record = (AcquisitionSnapshotRecord){sources->snapshots, {url, revision, NULL, origin}};
    sources->snapshots = record; sources->snapshot_count++;
    *output = &record->value; *created = true;
    return ACQUISITION_SOURCES_OK;
no_memory:
    return acquisition_sources_failure(issue, mapping->header, ACQUISITION_SOURCES_OUT_OF_MEMORY, "Cannot retain the exact snapshot request", NULL);
}
