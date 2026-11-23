/** @file
 * @brief Give every supplemental key one typed owner and fresh symbolic identity class.
 * [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates generated metadata from original physical identities and forbids
 * order-dependent collision merging.
 */
#include "internal.h"
#include <string.h>
SourceOperationWritesStatus source_operation_writes_extra(SourceOperationWrites *writes, SourceImageChange change,
    SourceOperationExtraOwner owner) {
    if (owner.kind < SOURCE_OPERATION_EXTRA_REMOVAL || owner.kind > SOURCE_OPERATION_EXTRA_CONFIGURATION
        || (owner.kind == SOURCE_OPERATION_EXTRA_REMOVAL ? !owner.owner.removal || change.kind != SOURCE_IMAGE_REMOVE
            : owner.kind == SOURCE_OPERATION_EXTRA_INITIALIZATION ? !owner.owner.initialization || change.kind == SOURCE_IMAGE_REMOVE
                : !owner.owner.configuration || change.kind == SOURCE_IMAGE_REMOVE)) { return SOURCE_OPERATION_WRITES_INVALID; }
    writes->failed_path = change.entry.path;
    if (!change.entry.path || writes->count == writes->capacity) { return SOURCE_OPERATION_WRITES_LIMIT; }
    for (size_t i = 0; i < writes->count; i++) {
        if (!strcmp(writes->changes[i].entry.path, change.entry.path)) { return SOURCE_OPERATION_WRITES_DUPLICATE; }
    }
    if (change.kind != SOURCE_IMAGE_REMOVE) {
        SourceCopyImageBuilder builder = {.original = &writes->inputs.observed->image, .changes = writes->changes, .count = writes->count};
        if (source_copy_image_identity(&builder, &change.entry.metadata) != SOURCE_COPY_IMAGE_OK) { return SOURCE_OPERATION_WRITES_LIMIT; }
    }
    writes->changes[writes->count] = change; writes->owners[writes->count++] = owner; return SOURCE_OPERATION_WRITES_OK;
}
