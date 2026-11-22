/** @file
 * @brief Retain one selected prepared version's exact configuration and optional guidance.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) fixes the selected copy;
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) requires original bytes
 * and distinguishes absent README guidance from an unreadable supplied entry.
 */
#include <string.h>
#include "internal.h"
/** Capture one exact metadata child, retaining absence only when optional. */
static AcquisitionClosureStatus metadata(SourceExpansion *expansion, const char *directory,
    const char *name, bool optional, const AcquisitionUnitEntry **output) {
    AcquisitionUnitEntry *entry = source_arena_allocate(&expansion->arena, sizeof(*entry));
    if (!entry) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    *entry = (AcquisitionUnitEntry){0};
    expansion->file_status = delivery_file_capture(&expansion->arena, &expansion->files, directory, name, &entry->file);
    if (expansion->file_status != SOURCE_OK) {
        return expansion->file_status == SOURCE_OUT_OF_MEMORY || expansion->file_status == SOURCE_LIMIT
            ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_PREPARATION_FAILED;
    }
    if (optional && entry->file.state == DELIVERY_FILE_ABSENT) { *output = NULL; return ACQUISITION_CLOSURE_OK; }
    if (entry->file.state != DELIVERY_FILE_PRESENT) {
        expansion->failed_path = entry->file.path;
        return ACQUISITION_CLOSURE_PREPARATION_FAILED;
    }
    *output = entry;
    return ACQUISITION_CLOSURE_OK;
}
AcquisitionClosureStatus source_expansion_version(SourceExpansion *expansion,
    const ModuleVersion *version, AcquisitionExpansion *fact) {
    if (!version) { return ACQUISITION_CLOSURE_OK; }
    SourceExpansionVersion *record = expansion->versions;
    while (record && record->version != version) { record = record->next; }
    if (!record) {
        record = source_arena_allocate(&expansion->arena, sizeof(*record));
        if (!record) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
        *record = (SourceExpansionVersion){.version = version,
            .source = {.version = version->logical_path, .directory = version->physical_path}};
        AcquisitionClosureStatus status = metadata(expansion, version->physical_path, "package.ini", false, &record->configuration);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        const SourceText *observed = record->configuration->file.content;
        if (source_length(observed) != source_length(version->configuration)
            || memcmp(source_bytes(observed), source_bytes(version->configuration), source_length(observed))) {
            expansion->failed_path = source_path(observed);
            return ACQUISITION_CLOSURE_CONFLICT;
        }
        status = metadata(expansion, version->physical_path, "README.md", true, &record->guidance);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        record->next = expansion->versions;
        expansion->versions = record;
    }
    fact->source = &record->source;
    fact->configuration = record->configuration;
    fact->guidance = record->guidance;
    return ACQUISITION_CLOSURE_OK;
}
