/** @file
 * @brief Prepare rooted original-scope module alias targets without altering keys or opaque source bytes.
 * [PKG-04](../../../../../../spec/project/packages.md#pkg-04)
 * fixes ordinary versus standard alias bases and nonshadowing;
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires actual final compiler/use checks after this parsed-field writer.
 */
#include "internal.h"
#include <string.h>
SourceOperationModuleWriteStatus source_operation_module_write_prepare(SourceOperationModuleWrite *write,
    const SourceOperationModuleWriteInputs *inputs) {
    if (!write || write->attempted) { return SOURCE_OPERATION_MODULE_INVALID; }
    write->attempted = true; write->status = source_operation_module_write_admit(write, inputs);
    if (write->status != SOURCE_OPERATION_MODULE_OK) { return write->status; }
    write->inputs = *inputs;
    write->document_status = configuration_document_parse(&write->document, inputs->clone->original->configuration, CONFIGURATION_MODULE);
    if (write->document_status != CONFIGURATION_OK) { return write->status = SOURCE_OPERATION_MODULE_DOCUMENT; }
    const char *identity = inputs->clone->original->logical_path;
    write->scope_status = configuration_module_aliases(&write->arena, &write->document, write->package,
        (const unsigned char *)identity, strlen(identity), &write->aliases, &write->scope_issue);
    if (write->scope_status != CONFIGURATION_SCOPE_OK) { return write->status = SOURCE_OPERATION_MODULE_ALIASES; }
    source_text_edits_init(&write->edits, write->texts.allocator, write->document.source); write->edits_initialized = true;
    write->aliases_status = source_copy_rebase_aliases(&write->edits, &write->document, write->aliases, &write->aliases_issue);
    if (write->aliases_status != SOURCE_COPY_ALIASES_OK) { return write->status = SOURCE_OPERATION_MODULE_ALIASES; }
    write->text_status = source_text_edits_apply(&write->edits, &write->texts, write->file->destination, &write->result);
    if (write->text_status != SOURCE_EDITS_OK) { return write->status = SOURCE_OPERATION_MODULE_TEXT; }
    write->valid = true; return SOURCE_OPERATION_MODULE_OK;
}
