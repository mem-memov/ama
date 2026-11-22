/** @file
 * @brief Build one complete immutable source/version/native protocol from admitted capture units.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves every selected identity;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires unconditional companions without native-role guesses.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Exact same-main suffix lookup consumes only structurally captured companions
 * under [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static const SourceText *companion(const SourceCapturedUnit *unit, const char *suffix) {
    const char *path = source_path(unit->source->text); size_t length = strlen(path);
    for (const AcquisitionUnitEntry *file = unit->files; file; file = file->next) {
        if (!strncmp(file->file.path, path, length) && !strcmp(file->file.path + length, suffix)) {
            return file->file.content;
        }
    }
    return NULL;
}
SourceCaptureStatus source_capture_tables(SourceArena *arena, const ModuleCatalog *catalog,
    const SourceCapturedUnit *units, DeliverySourceSnapshotView *output) {
    size_t count = 0;
    for (const SourceCapturedUnit *unit = units; unit; unit = unit->next) {
        if (count == SIZE_MAX) { return SOURCE_CAPTURE_LIMIT; } count++;
    }
    if (count > SIZE_MAX / sizeof(ModuleSnapshotEntry)) { return SOURCE_CAPTURE_LIMIT; }
    ModuleSnapshotEntry *entries = count ? source_arena_allocate(arena, count * sizeof(*entries)) : NULL;
    if (count && !entries) { return SOURCE_CAPTURE_MEMORY; }
    DeliverySourceCompanions *first = NULL, **tail = &first;
    size_t index = 0;
    for (const SourceCapturedUnit *unit = units; unit; unit = unit->next) {
        const ModuleSource *source = unit->source;
        entries[index++] = (ModuleSnapshotEntry){source->logical_path, source->text, source->device, source->inode, source->configuration};
        DeliverySourceCompanions *row = source_arena_allocate(arena, sizeof(*row));
        if (!row) { return SOURCE_CAPTURE_MEMORY; }
        const SourceText *body = companion(unit, ".c"), *header = companion(unit, ".h");
        *row = (DeliverySourceCompanions){.identity = source->logical_path,
            .body = body ? source_path(body) : NULL, .header = header ? source_path(header) : NULL,
            .abi = companion(unit, ".abi")};
        *tail = row; tail = &row->next;
    }
    DeliverySnapshotVersion *versions = NULL, **version_tail = &versions;
    for (const ModuleVersion *version = catalog->versions; version; version = version->next) {
        DeliverySnapshotVersion *row = source_arena_allocate(arena, sizeof(*row));
        if (!row) { return SOURCE_CAPTURE_MEMORY; }
        *row = (DeliverySnapshotVersion){.identity = version->logical_path,
            .directory = version->physical_path, .configuration = version->configuration};
        *version_tail = row; version_tail = &row->next;
    }
    *output = (DeliverySourceSnapshotView){entries, count, versions, first};
    return SOURCE_CAPTURE_OK;
}
