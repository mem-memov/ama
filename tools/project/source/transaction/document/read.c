/** @file
 * @brief Admit the closed image record only after exact schema and topology validation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never treats a parsed or partially decoded record as mutation authority.
 */
#include <string.h>
#include "internal.h"
/** One complete record has no ignored future fields or permissive version fallback. */
static SourceJsonStatus decode(SourceImageReader *reader, const SourceJsonValue *root) {
    const char *const names[] = {"format", "roots", "bounds", "entries"}; const SourceJsonValue *fields[4];
    SourceJsonStatus status = source_json_schema_fields(root, names, 4, fields, &reader->output->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *format = NULL; status = source_json_schema_cstring(fields[0], &format, &reader->output->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (strcmp(format, "ama.source-image.v2")) { return source_image_document_error(reader, fields[0], "Unsupported image record format", SOURCE_JSON_INVALID); }
    const ModuleRoot *roots = NULL; const SourceCatalogImageEntry *entries = NULL; size_t root_count = 0, entry_count = 0;
    status = source_image_document_roots(reader, fields[1], &roots, &root_count); if (status != SOURCE_JSON_OK) { return status; }
    SourceImageBounds bounds = {0};
    status = source_image_document_bounds(reader, fields[2], &bounds); if (status != SOURCE_JSON_OK) { return status; }
    status = source_image_document_entries(reader, fields[3], &entries, &entry_count); if (status != SOURCE_JSON_OK) { return status; }
    if (source_catalog_image_prepare(&reader->output->image, entries, entry_count, roots, root_count, bounds, &reader->output->image_issue) != SOURCE_IMAGE_OK) {
        return source_image_document_error(reader, root, "Invalid recorded image topology", SOURCE_JSON_INVALID);
    }
    return SOURCE_JSON_OK;
}
SourceJsonStatus source_image_document_read(SourceImageDocument *document, const SourceText *text) {
    if (!document || document->attempted || !text) { return SOURCE_JSON_INVALID; }
    document->attempted = true; SourceArena scratch; source_arena_init(&scratch, document->capture.arena.allocator);
    const SourceJsonValue *root = NULL;
    SourceJsonStatus status = source_json_parse(&scratch, text, &root, &document->issue);
    if (status == SOURCE_JSON_OK) { SourceImageReader reader = {document, &scratch}; status = decode(&reader, root); }
    source_arena_destroy(&scratch); document->status = status; document->valid = status == SOURCE_JSON_OK; return status;
}
