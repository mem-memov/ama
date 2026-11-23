/** @file
 * @brief Compose exact permitted selector additions/re-expression with owned carried alias rebasing.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves repository, key, comment and other bytes outside shared parsed writers.
 */
#include "internal.h"
SourceOperationConfigurationWriteStatus source_operation_configuration_write_prepare(SourceOperationConfigurationWrite *write,
    const SourceOperationConfigurationWriteInputs *inputs) {
    if (!write || write->attempted) { return SOURCE_OPERATION_CONFIGURATION_INVALID; }
    write->attempted = true; write->status = source_operation_configuration_write_admit(inputs);
    if (write->status != SOURCE_OPERATION_CONFIGURATION_OK) { return write->status; }
    write->inputs = *inputs;
    bool initialized = inputs->kind == SOURCE_OPERATION_CONFIGURATION_INITIALIZED;
    const SourceText *source = initialized ? inputs->after->document.source : inputs->before->document.source;
    source_text_edits_init(&write->edits, write->texts.allocator, source); write->edits_initialized = true;
    if (initialized) {
        write->configuration_status = source_copy_configuration_initialize(&write->arena, &write->edits,
            inputs->copy, inputs->mains, inputs->observed, inputs->preview, inputs->after, &write->roots, &write->configuration_issue);
    } else {
        write->configuration_status = source_copy_configuration_prepare(&write->arena, &write->edits,
            inputs->copy, inputs->mains, inputs->original, inputs->before, inputs->preview, inputs->after,
            inputs->kind == SOURCE_OPERATION_CONFIGURATION_CARRIED, inputs->versions, inputs->version_count,
            &write->roots, &write->configuration_issue);
    }
    if (write->configuration_status != SOURCE_CONFIGURATION_OK) { return write->status = SOURCE_OPERATION_CONFIGURATION_PERMISSION; }
    if (inputs->kind == SOURCE_OPERATION_CONFIGURATION_CARRIED) {
        write->aliases_status = source_copy_rebase_aliases(&write->edits, &inputs->before->document,
            inputs->before->aliases, &write->aliases_issue);
        if (write->aliases_status != SOURCE_COPY_ALIASES_OK) { return write->status = SOURCE_OPERATION_CONFIGURATION_ALIASES; }
    }
    write->text_status = source_text_edits_apply(&write->edits, &write->texts,
        source_path(inputs->after->document.source), &write->result);
    if (write->text_status != SOURCE_EDITS_OK) { return write->status = SOURCE_OPERATION_CONFIGURATION_TEXT; }
    write->valid = true; return SOURCE_OPERATION_CONFIGURATION_OK;
}
