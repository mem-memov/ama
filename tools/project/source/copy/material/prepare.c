/** @file
 * @brief Admit exact prepared overrides before publishing every mapped material row.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids silent input/output merging and preserves bytes outside authorized edits.
 */
#include "material.h"
#include <stdint.h>
#include <string.h>
/** Compare bytes independently of text ownership; capture and compiler-response
 * sources intentionally have distinct lifetimes and allocation identities. */
static bool equal_bytes(const SourceText *a, const SourceText *b) {
    return a && b && source_length(a) == source_length(b)
        && !memcmp(source_bytes(a), source_bytes(b), source_length(a));
}
/** Exact destination lookup never infers module identity from file suffixes. */
static const SourceFileMapping *destination(const SourceFileMap *map, const char *path) {
    for (const SourceFileMapping *row = map->first; row; row = row->next) {
        if (!strcmp(row->destination, path)) { return row; }
    }
    return NULL;
}
SourceMaterialStatus source_copy_material_prepare(SourceArena *arena, const SourceFileMap *map,
    const SourceGraphPreparedText *prepared, SourceCopyMaterial *output, SourceMaterialIssue *issue) {
    if (!issue) { return SOURCE_MATERIAL_INVALID; } *issue = (SourceMaterialIssue){0};
    if (!arena || !map || !map->ready || map->status != SOURCE_FILE_MAP_OK || !output) { return SOURCE_MATERIAL_INVALID; }
    for (const SourceGraphPreparedText *text = prepared; text; text = text->next) {
        issue->prepared = text;
        const SourceTextEdits *edits = text->edits;
        if (!edits || !edits->valid || !edits->attempted || edits->status != SOURCE_EDITS_OK
            || !edits->original || !edits->result) { return SOURCE_MATERIAL_INVALID; }
        const SourceFileMapping *row = destination(map, source_path(edits->result)); issue->mapping = row;
        if (!row || row->original->directory) { return SOURCE_MATERIAL_UNMAPPED; }
        if (strcmp(row->source, source_path(edits->original))
            || !equal_bytes(row->original->file.content, edits->original)) { return SOURCE_MATERIAL_ORIGINAL; }
        for (const SourceGraphPreparedText *previous = prepared; previous != text; previous = previous->next) {
            if (!strcmp(source_path(previous->edits->result), source_path(edits->result))) { return SOURCE_MATERIAL_DUPLICATE; }
        }
    }
    size_t count = 0;
    for (const SourceFileMapping *row = map->first; row; row = row->next) {
        if (count == SIZE_MAX / sizeof(SourceCopyMaterialEntry)) { return SOURCE_MATERIAL_LIMIT; } count++;
    }
    SourceCopyMaterialEntry *entries = count ? source_arena_allocate(arena, count * sizeof(*entries)) : NULL;
    if (count && !entries) { return SOURCE_MATERIAL_MEMORY; }
    size_t index = 0;
    for (const SourceFileMapping *row = map->first; row; row = row->next) {
        const SourceText *content = row->original->directory ? NULL : row->original->file.content;
        for (const SourceGraphPreparedText *text = prepared; text; text = text->next) {
            if (!strcmp(row->destination, source_path(text->edits->result))) { content = text->edits->result; break; }
        }
        entries[index++] = (SourceCopyMaterialEntry){row, content};
    }
    *output = (SourceCopyMaterial){entries, count}; *issue = (SourceMaterialIssue){0}; return SOURCE_MATERIAL_OK;
}
