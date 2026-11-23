/** @file
 * @brief Join only genuine copied module-alias producers over this exact copy and physical relation.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * leaves all other companions opaque, including native/build source text.
 */
#include "internal.h"
SourceOperationWritesStatus source_operation_writes_modules(SourceOperationWrites *writes) {
    for (size_t i = 0; i < writes->inputs.module_count; i++) {
        const SourceOperationModuleWrite *write = writes->inputs.modules[i];
        if (!write || !write->valid || !write->result || !write->edits.valid || write->result != write->edits.result
            || write->inputs.copy != writes->inputs.copy || write->inputs.mains != writes->inputs.mains
            || write->inputs.files != writes->inputs.files) { return SOURCE_OPERATION_WRITES_INVALID; }
        const SourceFileMapping *file = source_operation_writes_file(writes->inputs.files, source_path(write->result));
        if (!file || file != write->file) { return SOURCE_OPERATION_WRITES_SCOPE; }
        SourceOperationPreparedWrite proposed = {.kind = SOURCE_OPERATION_PREPARED_MODULE,
            .text = {.edits = &write->edits}, .owner.module = write};
        SourceOperationWritesStatus status = source_operation_writes_text(writes, &proposed);
        if (status != SOURCE_OPERATION_WRITES_OK) { return status; }
    }
    return SOURCE_OPERATION_WRITES_OK;
}
