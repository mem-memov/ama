/** @file
 * @brief Derive all known original active input consumers before reviewing a deletion cut.
 * [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * includes unused surviving definitions and shared declared assets, not just entry reachability;
 * [PKG-07](../../../../../../spec/project/packages.md#pkg-07) requires explicit asset metadata.
 */
#include <string.h>
#include "internal.h"
/** Exact admitted definition lookup; draft records have no invented executable input uses. */
static const DeliverySourceDefinition *definition(const SourceRemovalSelection *selection, const char *identity) {
    for (const DeliverySourceDefinition *item = selection->analysis->original.response.definitions; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return item; }
    }
    return NULL;
}
SourceRemovalSelectionStatus source_removal_selection_inputs(SourceRemovalSelection *selection, const SourceDeclaredGroup *declared) {
    SourceRemovalInputBuilder builder = {.selection = selection}; builder.tail = &builder.first;
    for (const DeliverySourceDefinition *owner = selection->analysis->original.response.definitions; owner; owner = owner->next) {
        const SourceCapturedUnit *unit = selection->tree->capture->units;
        while (unit && strcmp(unit->source->logical_path, owner->identity)) { unit = unit->next; }
        const DeliveryResponseSource *row = selection->analysis->original.response.sources;
        while (row && strcmp(row->identity, owner->identity)) { row = row->next; }
        if (!unit || !row) { return SOURCE_REMOVAL_SELECTION_INVALID; }
        SourceRemovalSelectionStatus status = source_removal_input_configuration(&builder, owner, unit, row);
        if (status == SOURCE_REMOVAL_SELECTION_OK) { status = source_removal_input_native(&builder, owner, unit, row); }
        if (status != SOURCE_REMOVAL_SELECTION_OK) { return status; }
    }
    for (const SourceDeclaredGroup *group = declared; group; group = group->next) {
        const DeliverySourceDefinition *owner = definition(selection, group->requester->source->logical_path);
        if (!owner) { continue; }
        for (const AcquisitionUnitEntry *file = group->files; file; file = file->next) {
            SourceRemovalSelectionStatus status = source_removal_input_append(&builder, owner, file->file.path, file->file.content);
            if (status != SOURCE_REMOVAL_SELECTION_OK) { return status; }
        }
    }
    selection->inputs = builder.first;
    return SOURCE_REMOVAL_SELECTION_OK;
}
