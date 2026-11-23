/** @file
 * @brief Prove copied package-table ownership through exact raw selection and source/destination bytes.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids carrying aliases/repositories from an unselected configuration.
 */
#include "internal.h"
#include <string.h>
/** Only a directly selected raw package configuration can carry its table. */
bool source_operation_configuration_carried(const SourceOperationConfigurationWriteInputs *inputs,
    const SourcePackageScope *original, const SourcePackageScope *final) {
    const char *before = source_path(original->document.source), *after = source_path(final->document.source);
    const AcquisitionUnitEntry *raw = inputs->copy->tree->members;
    while (raw && strcmp(raw->file.path, before)) { raw = raw->next; }
    if (!raw || raw->directory || source_length(raw->file.content) != source_length(original->document.source)
        || memcmp(source_bytes(raw->file.content), source_bytes(original->document.source), source_length(raw->file.content))) { return false; }
    for (const SourceFileMapping *file = inputs->files->first; file; file = file->next) {
        if (!strcmp(file->source, before) && !strcmp(file->destination, after) && !file->original->directory
            && source_length(file->original->file.content) == source_length(raw->file.content)
            && !memcmp(source_bytes(file->original->file.content), source_bytes(raw->file.content), source_length(raw->file.content))) { return true; }
    }
    return false;
}
