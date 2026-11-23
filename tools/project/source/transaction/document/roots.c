/** @file
 * @brief Decode ordered physical roots and exclusions in their own immutable epoch.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires recorded root settings, not inferred package prefixes or aliases.
 */
#include <string.h>
#include "internal.h"
SourceJsonStatus source_image_document_roots(SourceImageReader *reader, const SourceJsonValue *value,
    const ModuleRoot **output, size_t *size) {
    size_t count = 0; SourceJsonStatus status = source_image_document_array(reader, value, sizeof(ModuleRoot), &count);
    if (status != SOURCE_JSON_OK) { return status; }
    ModuleRoot *roots = count ? source_arena_allocate(&reader->output->capture.arena, count * sizeof(*roots)) : NULL;
    if (count && !roots) { return source_image_document_error(reader, value, "Cannot retain image roots", SOURCE_JSON_OUT_OF_MEMORY); }
    size_t i = 0;
    for (const SourceJsonValue *row = value->first; row; row = row->next, i++) {
        const char *const names[] = {"path", "kind", "excluded"}; const SourceJsonValue *fields[3];
        status = source_json_schema_fields(row, names, 3, fields, &reader->output->issue); if (status != SOURCE_JSON_OK) { return status; }
        roots[i] = (ModuleRoot){0}; const char *kind = NULL;
        status = source_image_document_path(reader, fields[0], false, &roots[i].path); if (status != SOURCE_JSON_OK) { return status; }
        status = source_json_schema_cstring(fields[1], &kind, &reader->output->issue); if (status != SOURCE_JSON_OK) { return status; }
        if (!strcmp(kind, "project")) { roots[i].kind = MODULE_ROOT_PROJECT; }
        else if (!strcmp(kind, "standard")) { roots[i].kind = MODULE_ROOT_STANDARD; }
        else { return source_image_document_error(reader, fields[1], "Unknown image root kind", SOURCE_JSON_INVALID); }
        size_t excluded = 0;
        status = source_image_document_array(reader, fields[2], sizeof(char *), &excluded); if (status != SOURCE_JSON_OK) { return status; }
        const char **entries = excluded ? source_arena_allocate(&reader->output->capture.arena, excluded * sizeof(*entries)) : NULL;
        if (excluded && !entries) { return source_image_document_error(reader, fields[2], "Cannot retain image exclusions", SOURCE_JSON_OUT_OF_MEMORY); }
        size_t j = 0;
        for (const SourceJsonValue *item = fields[2]->first; item; item = item->next, j++) {
            status = source_image_document_path(reader, item, true, &entries[j]); if (status != SOURCE_JSON_OK) { return status; }
        }
        roots[i].excluded = entries; roots[i].excluded_count = excluded;
    }
    *output = roots; *size = count; return SOURCE_JSON_OK;
}
