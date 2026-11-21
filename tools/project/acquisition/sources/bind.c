/** @file
 * @brief Intern one complete coherent source contract per logical version.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) compares URL, resolved
 * commit and normalized physical directory, allowing equal commits reached
 * through different revision names. [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * preserves prepared-copy authority without remote hole filling.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Prepared handles must belong to the operation's already selected view. */
static bool selected(const AcquisitionSources *sources, const ModulePackage *package) {
    for (const ModulePackage *entry = sources->prepared; entry; entry = entry->next) {
        if (entry == package) { return true; }
    }
    return false;
}
/** Equal remote contracts compare complete resolved keys, not request-order or
 * revision spelling. Prepared equality uses the one selected physical copy. */
static bool equivalent(const AcquisitionVersionSource *existing, const char *directory,
    const AcquisitionSnapshot *snapshot, const ModulePackage *local) {
    if (strcmp(existing->directory, directory)) { return false; }
    if (local) { return existing->local == local && !existing->snapshot; }
    return !existing->local && existing->snapshot &&
        !strcmp(existing->snapshot->url, snapshot->url) && !strcmp(existing->snapshot->commit, snapshot->commit);
}
AcquisitionSourcesStatus acquisition_sources_bind(AcquisitionSources *sources, const AcquisitionSourceRequest *request,
    const AcquisitionVersionSource **output, AcquisitionSourcesIssue *issue) {
    if (!sources || !request || !output || !issue) {
        return acquisition_sources_failure(issue, (SourceSpan){0}, ACQUISITION_SOURCES_INVALID, "Invalid version source binding", NULL);
    }
    const char *version = NULL, *directory = NULL;
    const AcquisitionSnapshot *snapshot = NULL;
    const ModulePackage *local = request->local;
    SourceSpan origin = request->origin;
    if (local) {
        if (!selected(sources, local) || request->location || request->snapshot) {
            return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_INVALID, "Prepared source handle does not belong to this selected view", NULL);
        }
        version = local->logical; directory = local->physical;
    } else {
        const AcquisitionLocation *location = request->location;
        if (!location || !request->snapshot || !acquisition_snapshot_record(sources, request->snapshot) ||
            !acquisition_source_text_equal(request->snapshot->url, location->route.mapping->url) ||
            !acquisition_source_text_equal(request->snapshot->revision, location->route.mapping->revision)) {
            return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_INVALID, "Remote request does not match this operation's selected snapshot", NULL);
        }
        snapshot = request->snapshot;
        if (!snapshot->commit) { return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_PENDING, "The selected snapshot has not been explicitly resolved", NULL); }
        version = location->version; directory = location->directory;
        for (const ModulePackage *entry = sources->prepared; entry; entry = entry->next) {
            if (!strcmp(entry->logical, version)) {
                return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_CONFLICT, "A prepared version cannot be filled from a remote snapshot", NULL);
            }
        }
        if (!source_span_valid(origin)) { origin = location->route.mapping->header; }
    }
    for (AcquisitionVersionRecord *entry = sources->versions; entry; entry = entry->next) {
        if (strcmp(entry->value.version, version)) { continue; }
        if (!equivalent(&entry->value, directory, snapshot, local)) {
            return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_CONFLICT,
                "This logical version already has an incompatible source contract", &entry->value);
        }
        *output = &entry->value; return ACQUISITION_SOURCES_OK;
    }
    if (sources->version_count == SIZE_MAX) {
        return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_LIMIT, "Version source count exceeds addressable storage", NULL);
    }
    AcquisitionVersionRecord *record = source_arena_allocate(&sources->arena, sizeof(*record));
    if (!record) { goto no_memory; }
    char *owned_version = source_arena_copy(&sources->arena, version);
    if (!owned_version) { goto no_memory; }
    char *owned_directory = source_arena_copy(&sources->arena, directory);
    if (!owned_directory) { goto no_memory; }
    *record = (AcquisitionVersionRecord){sources->versions, {owned_version, owned_directory, snapshot, local, origin}};
    sources->versions = record; sources->version_count++;
    *output = &record->value;
    return ACQUISITION_SOURCES_OK;
no_memory:
    return acquisition_sources_failure(issue, origin, ACQUISITION_SOURCES_OUT_OF_MEMORY, "Cannot retain the coherent version source", NULL);
}
