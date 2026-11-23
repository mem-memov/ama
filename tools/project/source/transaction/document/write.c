/** @file
 * @brief Emit exact image facts as a closed lossless byte/integer JSON record.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * persists raw opaque contents and original observation precision, never hash-only
 * equality claims or compiler execution state.
 */
#include <string.h>
#include "document.h"
#include "tools/output/json/json.h"
/** Filesystem paths are byte strings, including non-UTF8 data names. */
static bool path(ToolJsonWriter *writer, const char *value) {
    return tool_json_bytes(writer, (const unsigned char *)value, strlen(value));
}
/** Root order and exact exclusion bytes belong to the recorded domain. */
static bool roots(ToolJsonWriter *writer, const SourceCatalogImage *image) {
    if (!tool_json_raw(writer, "[")) { return false; }
    for (size_t i = 0; i < image->root_count; i++) {
        const ModuleRoot *root = &image->roots[i];
        if ((i && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"path\":") || !path(writer, root->path)
            || !tool_json_raw(writer, ",\"kind\":") || !tool_json_string(writer, root->kind == MODULE_ROOT_PROJECT ? "project" : "standard")
            || !tool_json_raw(writer, ",\"excluded\":[")) { return false; }
        for (size_t j = 0; j < root->excluded_count; j++) {
            if ((j && !tool_json_raw(writer, ",")) || !path(writer, root->excluded[j])) { return false; }
        }
        if (!tool_json_raw(writer, "]}")) { return false; }
    }
    return tool_json_raw(writer, "]");
}
/** Unknown metadata is distinct from a known zero inode or an empty file. */
static bool entry(ToolJsonWriter *writer, const SourceCatalogImageEntry *row) {
    if (!tool_json_raw(writer, "{\"path\":") || !path(writer, row->path) || !tool_json_raw(writer, ",\"metadata\":")) { return false; }
    if (row->metadata_captured) {
        if (!tool_json_raw(writer, "{\"device\":") || !tool_json_unsigned(writer, (uintmax_t)row->metadata.st_dev)
            || !tool_json_raw(writer, ",\"inode\":") || !tool_json_unsigned(writer, (uintmax_t)row->metadata.st_ino)
            || !tool_json_raw(writer, ",\"mode\":") || !tool_json_unsigned(writer, (uintmax_t)row->metadata.st_mode) || !tool_json_raw(writer, "}")) { return false; }
    } else if (!tool_json_raw(writer, "null")) { return false; }
    if (!tool_json_raw(writer, ",\"childrenComplete\":") || !tool_json_boolean(writer, row->children_complete) || !tool_json_raw(writer, ",\"content\":")) { return false; }
    if (row->content) {
        if (!tool_json_bytes(writer, source_bytes(row->content), source_length(row->content))) { return false; }
    } else if (!tool_json_raw(writer, "null")) { return false; }
    return tool_json_raw(writer, "}");
}
bool source_image_document_write(FILE *stream, const SourceCatalogImage *image) {
    if (!stream || !image || !image->valid) { return false; }
    ToolJsonWriter writer; tool_json_init(&writer, stream);
    if (!tool_json_raw(&writer, "{\"format\":\"ama.source-image.v2\",\"roots\":") || !roots(&writer, image)
        || !tool_json_raw(&writer, ",\"bounds\":[")) { return false; }
    for (size_t i = 0; i < source_image_bound_count(image); i++) {
        if ((i && !tool_json_raw(&writer, ",")) || !path(&writer, source_image_bound_path(image, i))) { return false; }
    }
    if (!tool_json_raw(&writer, "],\"entries\":[")) { return false; }
    for (size_t i = 0; i < image->count; i++) {
        if ((i && !tool_json_raw(&writer, ",")) || !entry(&writer, &image->entries[i])) { return false; }
    }
    return tool_json_raw(&writer, "]}") && tool_json_finish(&writer);
}
