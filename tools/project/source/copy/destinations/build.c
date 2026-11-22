/** @file
 * @brief Orchestrate immutable closure/explicit-map joins before publishing logical candidates.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires one coherent explicit/additional/test mapping; ordinary final typing,
 * physical discovery, configuration authority and transactions remain later gates.
 */
#include "internal.h"
SourceCopyDestinationsStatus source_copy_destinations_build(SourceCopyDestinations *result,
    const SourceCopyClosure *copy, const SourceCloneMap *explicit_map) {
    if (!result || result->attempted) { return SOURCE_COPY_DESTINATIONS_INVALID; }
    result->attempted = true;
    if (!copy || !copy->valid || !copy->tree || !copy->tree->valid || !copy->expansion.capture ||
        !copy->expansion.analysis || !copy->expansion.analysis->valid || !explicit_map ||
        explicit_map->capture != copy->expansion.capture || explicit_map->status != SOURCE_CLONE_OK) {
        return result->status = SOURCE_COPY_DESTINATIONS_INVALID;
    }
    result->copy = copy; result->explicit_map = explicit_map;
    SourceCopyDestinationWork work = {0};
    result->status = source_copy_destination_units(result, &work);
    if (result->status != SOURCE_COPY_DESTINATIONS_OK) { return result->status; }
    result->status = source_copy_destination_flows(result, &work);
    if (result->status != SOURCE_COPY_DESTINATIONS_OK) { return result->status; }
    result->version_status = source_version_plan(&result->arena, work.units, result->count,
        work.flows, work.flow_count, &work.assignments, &result->version_issue);
    if (result->version_status != SOURCE_VERSION_OK) {
        if ((result->version_status == SOURCE_VERSION_CONFLICT || result->version_status == SOURCE_VERSION_UNRESOLVED) &&
            result->version_issue.unit < result->count) { result->failed_role = work.entries[result->version_issue.unit].role; }
        return result->status = result->version_status == SOURCE_VERSION_MEMORY
            ? SOURCE_COPY_DESTINATIONS_MEMORY : SOURCE_COPY_DESTINATIONS_VERSIONS;
    }
    result->status = source_copy_destination_names(result, &work);
    if (result->status != SOURCE_COPY_DESTINATIONS_OK) { return result->status; }
    result->entries = work.entries; result->failed_role = NULL; result->valid = true;
    return SOURCE_COPY_DESTINATIONS_OK;
}
