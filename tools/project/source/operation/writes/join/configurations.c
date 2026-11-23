/** @file
 * @brief Join recognized package recipe results without permitting arbitrary extra configuration bytes.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * distinguishes carried tables, unchanged existing environments and newly empty
 * versions; only their typed producers supply output bytes.
 */
#include "internal.h"
#include <string.h>
/** Exact original regular metadata/bytes are required for an existing-file update. */
static const SourceCatalogImageEntry *original(const SourceOperationWrites *writes, const char *path) {
    for (size_t i = 0; i < writes->inputs.observed->image.count; i++) {
        const SourceCatalogImageEntry *row = &writes->inputs.observed->image.entries[i];
        if (!strcmp(row->path, path)) { return row; }
    }
    return NULL;
}
/** Configuration kind requires exactly one registered initializer, never a
 * same-named empty object or another writer's initialization row. */
static SourceOperationWritesStatus initialized(SourceOperationWrites *writes, const SourceOperationConfigurationWrite *write) {
    const SourceVersionInitialization *initialization = write->inputs.initialization;
    bool registered = false;
    for (size_t i = 0; i < writes->inputs.initialization_count; i++) {
        if (writes->inputs.initializations[i] == initialization) { registered = true; }
    }
    if (!registered) { return SOURCE_OPERATION_WRITES_SCOPE; }
    const char *path = source_path(write->result);
    for (size_t i = 0; i < writes->count; i++) {
        if (strcmp(writes->changes[i].entry.path, path)) { continue; }
        if (writes->owners[i].kind != SOURCE_OPERATION_EXTRA_INITIALIZATION
            || writes->owners[i].owner.initialization != initialization || writes->changes[i].kind != SOURCE_IMAGE_ADD
            || !S_ISREG(writes->changes[i].entry.metadata.st_mode)) { return SOURCE_OPERATION_WRITES_DUPLICATE; }
        writes->changes[i].entry.content = write->result;
        writes->owners[i] = (SourceOperationExtraOwner){.kind = SOURCE_OPERATION_EXTRA_CONFIGURATION, .owner.configuration = write};
        return SOURCE_OPERATION_WRITES_OK;
    }
    return SOURCE_OPERATION_WRITES_SCOPE;
}
SourceOperationWritesStatus source_operation_writes_configurations(SourceOperationWrites *writes) {
    for (size_t i = 0; i < writes->inputs.configuration_count; i++) {
        const SourceOperationConfigurationWrite *write = writes->inputs.configurations[i];
        if (!write || !write->valid || !write->result || !write->edits.valid || write->result != write->edits.result
            || write->inputs.copy != writes->inputs.copy || write->inputs.mains != writes->inputs.mains
            || write->inputs.files != writes->inputs.files || write->inputs.observed != writes->inputs.observed) { return SOURCE_OPERATION_WRITES_INVALID; }
        const char *path = source_path(write->result); writes->failed_path = path;
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(source_path(writes->inputs.configurations[j]->result), path)) { return SOURCE_OPERATION_WRITES_DUPLICATE; }
        }
        if (write->inputs.kind == SOURCE_OPERATION_CONFIGURATION_CARRIED) {
            SourceOperationPreparedWrite proposed = {.kind = SOURCE_OPERATION_PREPARED_CONFIGURATION,
                .text = {.edits = &write->edits}, .owner.configuration = write};
            SourceOperationWritesStatus status = source_operation_writes_text(writes, &proposed);
            if (status != SOURCE_OPERATION_WRITES_OK) { return status; }
        } else if (write->inputs.kind == SOURCE_OPERATION_CONFIGURATION_INITIALIZED) {
            SourceOperationWritesStatus status = initialized(writes, write);
            if (status != SOURCE_OPERATION_WRITES_OK) { return status; }
        } else if (write->inputs.kind == SOURCE_OPERATION_CONFIGURATION_EXISTING) {
            const SourceCatalogImageEntry *row = original(writes, path);
            if (!row || !row->metadata_captured || !S_ISREG(row->metadata.st_mode) || !row->content
                || source_length(row->content) != source_length(write->edits.original)
                || strcmp(row->path, source_path(write->edits.original))
                || memcmp(source_bytes(row->content), source_bytes(write->edits.original), source_length(row->content))) { return SOURCE_OPERATION_WRITES_MATERIAL; }
            if (source_length(write->result) == source_length(row->content)
                && !memcmp(source_bytes(write->result), source_bytes(row->content), source_length(row->content))) { continue; }
            SourceImageChange change = {SOURCE_IMAGE_UPDATE, *row}; change.entry.content = write->result;
            SourceOperationExtraOwner owner = {.kind = SOURCE_OPERATION_EXTRA_CONFIGURATION, .owner.configuration = write};
            SourceOperationWritesStatus status = source_operation_writes_extra(writes, change, owner);
            if (status != SOURCE_OPERATION_WRITES_OK) { return status; }
        } else { return SOURCE_OPERATION_WRITES_INVALID; }
    }
    return SOURCE_OPERATION_WRITES_OK;
}
