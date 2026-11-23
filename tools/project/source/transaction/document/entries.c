/** @file
 * @brief Decode exact metadata and opaque content without interpreting language roles.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves unknown precision and integer identities separately from empty bytes.
 */
#include "internal.h"
/** Filesystem identity integers must roundtrip through actual host field types. */
static SourceJsonStatus metadata(SourceImageReader *reader, const SourceJsonValue *value, SourceCatalogImageEntry *row) {
    if (value->kind == SOURCE_JSON_NULL) { return SOURCE_JSON_OK; }
    const char *const names[] = {"device", "inode", "mode"}; const SourceJsonValue *fields[3];
    SourceJsonStatus status = source_json_schema_fields(value, names, 3, fields, &reader->output->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    uintmax_t values[3];
    for (size_t i = 0; i < 3; i++) {
        status = source_json_schema_unsigned(fields[i], &values[i], &reader->output->issue); if (status != SOURCE_JSON_OK) { return status; }
    }
    row->metadata.st_dev = (dev_t)values[0]; row->metadata.st_ino = (ino_t)values[1]; row->metadata.st_mode = (mode_t)values[2];
    if ((uintmax_t)row->metadata.st_dev != values[0] || (uintmax_t)row->metadata.st_ino != values[1] || (uintmax_t)row->metadata.st_mode != values[2]) {
        return source_image_document_error(reader, value, "Image metadata does not fit native field types", SOURCE_JSON_INVALID);
    }
    row->metadata_captured = true; return SOURCE_JSON_OK;
}
SourceJsonStatus source_image_document_entries(SourceImageReader *reader, const SourceJsonValue *value,
    const SourceCatalogImageEntry **output, size_t *size) {
    size_t count = 0; SourceJsonStatus status = source_image_document_array(reader, value, sizeof(SourceCatalogImageEntry), &count);
    if (status != SOURCE_JSON_OK) { return status; }
    SourceCatalogImageEntry *rows = count ? source_arena_allocate(&reader->output->capture.arena, count * sizeof(*rows)) : NULL;
    if (count && !rows) { return source_image_document_error(reader, value, "Cannot retain image entries", SOURCE_JSON_OUT_OF_MEMORY); }
    size_t i = 0;
    for (const SourceJsonValue *item = value->first; item; item = item->next, i++) {
        const char *const names[] = {"path", "metadata", "childrenComplete", "content"}; const SourceJsonValue *fields[4];
        status = source_json_schema_fields(item, names, 4, fields, &reader->output->issue); if (status != SOURCE_JSON_OK) { return status; }
        rows[i] = (SourceCatalogImageEntry){0};
        status = source_image_document_path(reader, fields[0], false, &rows[i].path); if (status != SOURCE_JSON_OK) { return status; }
        status = metadata(reader, fields[1], &rows[i]); if (status != SOURCE_JSON_OK) { return status; }
        if (fields[2]->kind != SOURCE_JSON_BOOLEAN) { return source_image_document_error(reader, fields[2], "Image completeness must be boolean", SOURCE_JSON_INVALID); }
        rows[i].children_complete = fields[2]->boolean;
        if (fields[3]->kind != SOURCE_JSON_NULL) {
            SourceJsonText bytes;
            status = source_json_schema_bytes(reader->scratch, fields[3], &bytes, &reader->output->issue); if (status != SOURCE_JSON_OK) { return status; }
            if (source_manager_add(&reader->output->capture.sources, rows[i].path, bytes.bytes, bytes.length, &rows[i].content) != SOURCE_OK) {
                return source_image_document_error(reader, fields[3], "Cannot retain image content bytes", SOURCE_JSON_OUT_OF_MEMORY);
            }
        }
    }
    *output = rows; *size = count; return SOURCE_JSON_OK;
}
