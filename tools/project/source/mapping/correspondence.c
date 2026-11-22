/** @file
 * @brief Join complete original-index, physical-input and nominal-copy relations without guessing roles.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps every copied definition in one injective relation regardless of why its
 * bytes were selected. A main selected as data is still an indexed definition.
 */
#include <string.h>
#include "correspondence.h"
SourceCorrespondenceStatus source_mapping_correspondence(const SourceCloneMap *clones,
    const SourceFileMap *files, SourceCorrespondenceIssue *issue) {
    if (!issue) { return SOURCE_CORRESPONDENCE_INVALID; }
    *issue = (SourceCorrespondenceIssue){0};
    if (!clones || !clones->ready || !clones->capture || !clones->capture->valid
        || !files || !files->ready) { return SOURCE_CORRESPONDENCE_INVALID; }
    for (const SourceCapturedUnit *unit = clones->capture->units; unit; unit = unit->next) {
        const SourceCloneMapping *clone = clones->first;
        while (clone && clone->original != unit->source) { clone = clone->next; }
        const char *path = source_path(unit->source->text);
        const SourceFileMapping *file = files->first;
        while (file && strcmp(file->source, path)) { file = file->next; }
        if (!clone && !file) { continue; }
        *issue = (SourceCorrespondenceIssue){unit, file, clone};
        if (!clone || !file) { return SOURCE_CORRESPONDENCE_MISSING; }
        if (strcmp(file->destination, clone->path) || file->original->directory
            || source_length(file->original->file.content) != source_length(unit->source->text)
            || memcmp(source_bytes(file->original->file.content), source_bytes(unit->source->text), source_length(unit->source->text))) {
            return SOURCE_CORRESPONDENCE_CHANGED;
        }
    }
    *issue = (SourceCorrespondenceIssue){0};
    return SOURCE_CORRESPONDENCE_OK;
}
