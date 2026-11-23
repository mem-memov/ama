/** @file
 * @brief Prove that each introduced unit was an untouched ignored ordinary supplier in the original domain.
 * [PKG-02](../../../../../../spec/project/packages.md#pkg-02) is the only permitted
 * suppression; [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids augmenting original semantic coverage after the fact.
 */
#include "internal.h"
#include "../../../capture/preservation.h"
#include <string.h>
/** Exact known regular bytes certify a configuration; unavailable is not empty. */
static bool configuration(const SourceCatalogImage *image, const SourceText *text) {
    if (!text) { return false; }
    for (size_t i = 0; i < image->count; i++) {
        const SourceCatalogImageEntry *row = &image->entries[i];
        if (strcmp(row->path, source_path(text))) { continue; }
        return row->metadata_captured && S_ISREG(row->metadata.st_mode) && row->content
            && source_length(row->content) == source_length(text)
            && !memcmp(source_bytes(row->content), source_bytes(text), source_length(text));
    }
    return false;
}
SourceIntroductionStatus source_copy_introduction_origin(SourceArena *arena, const SourceObservedImage *observed,
    const SourceCapturedUnit *unit, SourceIntroductionIssue *issue) {
    const ModuleSource *source = unit->source;
    const ModuleVersion *version = source->version;
    issue->unit = unit; issue->path = source_path(source->text);
    if (!version) { return SOURCE_INTRODUCTION_ORIGIN; }
    const ModuleVersion *old = observed->original->catalog.versions;
    while (old && strcmp(old->logical_path, version->logical_path)) { old = old->next; }
    if (!old || !strcmp(old->physical_path, version->physical_path)) { return SOURCE_INTRODUCTION_ORIGIN; }
    const char *paths[] = {version->physical_path, source_path(source->text)};
    const char *names[] = {version->logical_path, source->logical_path};
    const ModulePathExtent extents[] = {MODULE_PATH_VERSION, MODULE_PATH_MODULE};
    for (size_t i = 0; i < 2; i++) {
        SourcePlacedIdentity placement; issue->path = paths[i];
        issue->identity_status = source_placement_identity(arena, observed->original->roots,
            observed->original->root_count, paths[i], extents[i], &placement);
        if (issue->identity_status == SOURCE_IDENTITY_MEMORY) { return SOURCE_INTRODUCTION_MEMORY; }
        if (issue->identity_status != SOURCE_IDENTITY_OK || strcmp(placement.identity, names[i])) { return SOURCE_INTRODUCTION_ORIGIN; }
    }
    issue->path = source_path(source->text);
    if (!source_captured_unit_preserved(unit, &observed->image)) { return SOURCE_INTRODUCTION_MATERIAL; }
    issue->path = source_path(version->configuration);
    return configuration(&observed->image, version->configuration) ? SOURCE_INTRODUCTION_OK : SOURCE_INTRODUCTION_MATERIAL;
}
