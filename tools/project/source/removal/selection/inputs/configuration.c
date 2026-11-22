/** @file
 * @brief Preserve original module and selected-version configuration inputs of surviving definitions.
 * [PKG-02](../../../../../../spec/project/packages.md#pkg-02) fixes one selected
 * version; [PKG-03](../../../../../../spec/project/packages.md#pkg-03) gives
 * captured configuration authority; [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * forbids deleting a surviving owner's supported inputs.
 */
#include "internal.h"
SourceRemovalSelectionStatus source_removal_input_configuration(SourceRemovalInputBuilder *builder,
    const DeliverySourceDefinition *definition, const SourceCapturedUnit *unit, const DeliveryResponseSource *row) {
    if (!row->configuration_captured) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    SourceRemovalSelectionStatus status = SOURCE_REMOVAL_SELECTION_OK;
    if (row->configuration) {
        status = source_removal_input_append(builder, definition, source_path(row->configuration), row->configuration);
    }
    if (status != SOURCE_REMOVAL_SELECTION_OK || !unit->source->version) { return status; }
    const SourceText *configuration = unit->source->version->configuration;
    if (!configuration) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    return source_removal_input_append(builder, definition, source_path(configuration), configuration);
}
