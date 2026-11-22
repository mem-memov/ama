/** @file
 * @brief Retain typed native interface/build inputs without inspecting or executing their contents.
 * [BUILD-01](../../../../../../spec/implementation/application-builds.md#build-01)
 * binds the authored CMake companion; [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves required native files; optional README guidance is not a compilation input.
 */
#include <string.h>
#include "internal.h"
/** Resolve only a known typed native path within its original complete companion capture. */
static SourceRemovalSelectionStatus required(SourceRemovalInputBuilder *builder,
    const DeliverySourceDefinition *definition, const SourceCapturedUnit *unit, const char *path) {
    if (!path) { return SOURCE_REMOVAL_SELECTION_OK; }
    for (const AcquisitionUnitEntry *file = unit->files; file; file = file->next) {
        if (!strcmp(file->file.path, path)) {
            return source_removal_input_append(builder, definition, path, file->file.content);
        }
    }
    builder->selection->failed_path = path;
    return SOURCE_REMOVAL_SELECTION_INVALID;
}
SourceRemovalSelectionStatus source_removal_input_native(SourceRemovalInputBuilder *builder,
    const DeliverySourceDefinition *definition, const SourceCapturedUnit *unit, const DeliveryResponseSource *row) {
    if (!row->native_captured) { return SOURCE_REMOVAL_SELECTION_OK; }
    const char *paths[] = {row->native_body, row->native_header, row->native_abi ? source_path(row->native_abi) : NULL};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        SourceRemovalSelectionStatus status = required(builder, definition, unit, paths[index]);
        if (status != SOURCE_REMOVAL_SELECTION_OK) { return status; }
    }
    const char *main = source_path(unit->source->text); size_t length = strlen(main);
    for (const AcquisitionUnitEntry *file = unit->files; file; file = file->next) {
        if (!strncmp(file->file.path, main, length) && !strcmp(file->file.path + length, ".cmake")) {
            return source_removal_input_append(builder, definition, file->file.path, file->file.content);
        }
    }
    return SOURCE_REMOVAL_SELECTION_OK;
}
