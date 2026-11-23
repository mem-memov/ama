/** @file
 * @brief Bind configuration authority to real destination material and original scope identities.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids importing an unselected package table or patching unrelated source packages.
 */
#include "internal.h"
#include <string.h>
/** Scope pointer membership precedes every dereference of caller-supplied rows. */
static bool owns(const SourcePackageEnvironment *environment, const SourcePackageScope *scope) {
    if (!environment || !environment->valid) { return false; }
    for (const SourcePackageScope *row = environment->first; row; row = row->next) { if (row == scope) { return true; } }
    return false;
}
/** An actual P destination, including an empty directory, designates its existing
 * containing version; an unrelated configured scope acquires no write authority. */
static bool destination(const SourceOperationConfigurationWriteInputs *inputs) {
    for (const SourceFileMapping *file = inputs->files->first; file; file = file->next) {
        if (source_image_below(file->destination, inputs->after->version->physical_path)) { return true; }
    }
    return false;
}
SourceOperationConfigurationWriteStatus source_operation_configuration_write_admit(const SourceOperationConfigurationWriteInputs *inputs) {
    if (!inputs || inputs->kind < SOURCE_OPERATION_CONFIGURATION_EXISTING || inputs->kind > SOURCE_OPERATION_CONFIGURATION_INITIALIZED
        || !inputs->copy || !inputs->copy->valid || !inputs->mains || !inputs->mains->ready
        || inputs->mains->capture != inputs->copy->tree->capture || !inputs->files || !inputs->files->ready
        || !inputs->observed || !inputs->observed->valid || !inputs->observed->sealed
        || inputs->observed->original != inputs->copy->tree->capture || !owns(inputs->preview, inputs->after)
        || (inputs->version_count && !inputs->versions)) { return SOURCE_OPERATION_CONFIGURATION_INVALID; }
    if (!destination(inputs)) { return SOURCE_OPERATION_CONFIGURATION_SCOPE; }
    if (inputs->kind != SOURCE_OPERATION_CONFIGURATION_CARRIED) {
        for (const SourceFileMapping *file = inputs->files->first; file; file = file->next) {
            if (!strcmp(file->destination, source_path(inputs->after->document.source))) { return SOURCE_OPERATION_CONFIGURATION_SCOPE; }
        }
    }
    if (inputs->kind == SOURCE_OPERATION_CONFIGURATION_INITIALIZED) {
        const SourceVersionInitialization *initialization = inputs->initialization;
        if (inputs->original || inputs->before || inputs->versions || inputs->version_count || !initialization
            || initialization->original != inputs->observed || !initialization->configuration
            || !initialization->version.identity || !initialization->version.physical
            || source_length(initialization->configuration) || source_length(inputs->after->document.source)
            || strcmp(source_path(initialization->configuration), source_path(inputs->after->document.source))
            || strcmp(initialization->version.identity, inputs->after->version->logical_path)
            || strcmp(initialization->version.physical, inputs->after->version->physical_path)) { return SOURCE_OPERATION_CONFIGURATION_SCOPE; }
        return SOURCE_OPERATION_CONFIGURATION_OK;
    }
    if (inputs->initialization || !owns(inputs->original, inputs->before)
        || inputs->original->catalog != &inputs->copy->tree->capture->catalog) { return SOURCE_OPERATION_CONFIGURATION_INVALID; }
    if (inputs->kind == SOURCE_OPERATION_CONFIGURATION_CARRIED) {
        return source_operation_configuration_carried(inputs, inputs->before, inputs->after)
            && source_operation_configuration_versions(inputs) ? SOURCE_OPERATION_CONFIGURATION_OK : SOURCE_OPERATION_CONFIGURATION_SCOPE;
    }
    return inputs->versions || inputs->version_count
        || strcmp(inputs->before->version->logical_path, inputs->after->version->logical_path)
        || strcmp(inputs->before->version->physical_path, inputs->after->version->physical_path)
        || strcmp(source_path(inputs->before->document.source), source_path(inputs->after->document.source))
        ? SOURCE_OPERATION_CONFIGURATION_SCOPE : SOURCE_OPERATION_CONFIGURATION_OK;
}
