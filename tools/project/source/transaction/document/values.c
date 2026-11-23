/** @file
 * @brief Decode exact physical strings and bounded row counts without repair.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * cannot identify recoverable material through truncated or normalized path bytes.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
SourceJsonStatus source_image_document_error(SourceImageReader *reader, const SourceJsonValue *value,
    const char *message, SourceJsonStatus status) {
    reader->output->issue = (SourceJsonIssue){.origin = value ? value->origin : (SourceSpan){0}, .message = message}; return status;
}
SourceJsonStatus source_image_document_path(SourceImageReader *reader, const SourceJsonValue *value, bool empty, const char **output) {
    SourceJsonText bytes;
    SourceJsonStatus status = source_json_schema_bytes(reader->scratch, value, &bytes, &reader->output->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if ((!empty && !bytes.length) || memchr(bytes.bytes, 0, bytes.length)) { return source_image_document_error(reader, value, "Image path bytes must be NUL-free and nonempty", SOURCE_JSON_INVALID); }
    const char *copy = source_arena_copy(&reader->output->capture.arena, (const char *)bytes.bytes);
    if (!copy) { return source_image_document_error(reader, value, "Cannot retain image path bytes", SOURCE_JSON_OUT_OF_MEMORY); }
    *output = copy; return SOURCE_JSON_OK;
}
SourceJsonStatus source_image_document_array(SourceImageReader *reader, const SourceJsonValue *value, size_t width, size_t *output) {
    if (value->kind != SOURCE_JSON_ARRAY) { return source_image_document_error(reader, value, "Image table must be an array", SOURCE_JSON_INVALID); }
    size_t count = 0;
    for (const SourceJsonValue *row = value->first; row; row = row->next) {
        if (count == SIZE_MAX / width) { return source_image_document_error(reader, value, "Image table exceeds addressable storage", SOURCE_JSON_OUT_OF_MEMORY); } count++;
    }
    *output = count; return SOURCE_JSON_OK;
}
