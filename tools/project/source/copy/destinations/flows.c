/** @file
 * @brief Construct exact active copy-version demands with deepest associated-test ownership.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * makes associated tests follow the deepest source map; ordinary additional
 * dependencies follow their actual requiring copied units. Topology retention
 * alone is not an instruction to create an active copy at the old parent name.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Requesters are exact retained role handles, not similarly named source files. */
static size_t role_index(const SourceCopyDestinations *result, const SourceCopyDestinationWork *work,
    const AcquisitionRoleRecord *role) {
    for (size_t i = 0; i < result->count; i++) { if (work->entries[i].role == role) { return i; } }
    return SIZE_MAX;
}
/** Canonical strict directory nesting has a unique longest containing prefix. */
static size_t associated_owner(const SourceCopyDestinations *result,
    const SourceCopyDestinationWork *work, const char *identity) {
    size_t owner = SIZE_MAX, depth = 0, length = strlen(identity);
    for (size_t i = 0; i < result->count; i++) {
        if (!work->associations[i]) { continue; }
        size_t prefix = strlen(work->associations[i]);
        if (prefix > depth && length > prefix && identity[prefix] == '/' &&
            !memcmp(identity, work->associations[i], prefix)) { owner = i; depth = prefix; }
    }
    return owner;
}
/** Count and fill the same immutable flow list in two exact passes. */
static bool enumerate(const SourceCopyDestinations *result, SourceCopyDestinationWork *work, bool fill) {
    size_t count = 0;
    for (size_t target = 0; target < result->count; target++) {
        if (work->units[target].fixed_version) { continue; }
        size_t owner = associated_owner(result, work, work->entries[target].source->logical_path);
        if (owner != SIZE_MAX) {
            if (count == SIZE_MAX) { return false; }
            if (fill) { work->flows[count] = (SourceVersionFlow){owner, target}; }
            count++; continue;
        }
        for (const AcquisitionReason *reason = work->entries[target].role->reasons; reason; reason = reason->next) {
            if (!reason->requester || reason->kind == ACQUISITION_TOPOLOGY) { continue; }
            size_t requester = role_index(result, work, reason->requester);
            if (requester == SIZE_MAX) { continue; }
            if (count == SIZE_MAX) { return false; }
            if (fill) { work->flows[count] = (SourceVersionFlow){requester, target}; }
            count++;
        }
    }
    work->flow_count = count;
    return true;
}
SourceCopyDestinationsStatus source_copy_destination_flows(SourceCopyDestinations *result,
    SourceCopyDestinationWork *work) {
    if (!enumerate(result, work, false) || work->flow_count > SIZE_MAX / sizeof(*work->flows)) {
        return SOURCE_COPY_DESTINATIONS_MEMORY;
    }
    if (work->flow_count) {
        work->flows = source_arena_allocate(&result->arena, work->flow_count * sizeof(*work->flows));
        if (!work->flows) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
        if (!enumerate(result, work, true)) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
    }
    return SOURCE_COPY_DESTINATIONS_OK;
}
