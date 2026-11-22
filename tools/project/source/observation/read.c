/** @file
 * @brief Capture original bytes once under observed regular-file metadata.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains immutable observations without replacing failed snapshots; opaque native
 * companion bytes are not validated or executed by this capture operation.
 */
#include "internal.h"
SourceStatus source_observation_read(void *context, const char *path, const SourceText **output) {
    SourceObservedImage *observed = context;
    if (observed->sealed || observed->status != SOURCE_OBSERVATION_OK) { return SOURCE_INVALID_ARGUMENT; }
    SourceObservedEntry *entry = source_observation_find(observed, path);
    if (!entry || !entry->value.metadata_captured || !S_ISREG(entry->value.metadata.st_mode)) {
        observed->status = SOURCE_OBSERVATION_INVALID; observed->failed_path = path; return SOURCE_INVALID_ARGUMENT;
    }
    if (!entry->value.content) {
        const SourceText *content = NULL;
        SourceStatus status = source_manager_load(&observed->files, entry->value.path, &content);
        if (status != SOURCE_OK) {
            observed->status = status == SOURCE_OUT_OF_MEMORY ? SOURCE_OBSERVATION_MEMORY : SOURCE_OBSERVATION_PHYSICAL;
            observed->file_status = status; observed->failed_path = entry->value.path; return status;
        }
        entry->value.content = content;
    }
    *output = entry->value.content; return SOURCE_OK;
}
