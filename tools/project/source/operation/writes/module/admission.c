/** @file
 * @brief Admit only a real copied active module's registered local configuration at its exact mapped unit path.
 * [SYN-04](../../../../../../spec/language/source-and-names.md#syn-04)
 * leaves drafts opaque; [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids granting edit authority by a matching filename or borrowed foreign table.
 */
#include "internal.h"
#include <string.h>
SourceOperationModuleWriteStatus source_operation_module_write_admit(SourceOperationModuleWrite *write,
    const SourceOperationModuleWriteInputs *inputs) {
    if (!inputs || !inputs->copy || !inputs->copy->valid || !inputs->mains || !inputs->mains->ready
        || inputs->mains->capture != inputs->copy->tree->capture || !inputs->files || !inputs->files->ready
        || (inputs->original && (!inputs->original->valid || inputs->original->catalog != &inputs->mains->capture->catalog))) {
        return SOURCE_OPERATION_MODULE_INVALID;
    }
    const SourceCloneMapping *clone = inputs->mains->first;
    while (clone && clone != inputs->clone) { clone = clone->next; }
    if (!clone) { return SOURCE_OPERATION_MODULE_INVALID; }
    const DeliverySourceDefinition *definition = inputs->copy->expansion.analysis->original.response.definitions;
    while (definition && strcmp(definition->identity, clone->original->logical_path)) { definition = definition->next; }
    if (!definition) { return SOURCE_OPERATION_MODULE_DRAFT; }
    const ModuleSource *source = clone->original;
    if (!source->configuration_loaded || !source->configuration || !source->configuration_path
        || strcmp(source_path(source->configuration), source->configuration_path)) { return SOURCE_OPERATION_MODULE_SCOPE; }
    if (source->version) {
        if (!inputs->original) { return SOURCE_OPERATION_MODULE_SCOPE; }
        const SourcePackageScope *scope = inputs->original->first;
        while (scope && scope->version != source->version) { scope = scope->next; }
        if (!scope) { return SOURCE_OPERATION_MODULE_SCOPE; } write->package = scope->aliases;
    }
    const SourceFileMapping *file = inputs->files->first;
    while (file && strcmp(file->source, source->configuration_path)) { file = file->next; }
    if (!file || file->original->directory || source_length(file->original->file.content) != source_length(source->configuration)
        || memcmp(source_bytes(file->original->file.content), source_bytes(source->configuration), source_length(source->configuration))) {
        return SOURCE_OPERATION_MODULE_SCOPE;
    }
    const char *expected = NULL;
    write->path_status = source_unit_member_path(&write->arena, source_path(source->text), clone->path,
        source->configuration_path, false, &expected);
    if (write->path_status != SOURCE_UNIT_PATH_OK || strcmp(file->destination, expected)) { return SOURCE_OPERATION_MODULE_SCOPE; }
    write->file = file; return SOURCE_OPERATION_MODULE_OK;
}
