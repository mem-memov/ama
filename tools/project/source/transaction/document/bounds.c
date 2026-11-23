/** @file
 * @brief Retain physical observation boundaries independently of named module roots.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires recovery to retain its exact observation envelope without making a
 * stable parent into a module-identity root. The output epoch owns decoded keys.
 */
#include "internal.h"
SourceJsonStatus source_image_document_bounds(SourceImageReader *reader, const SourceJsonValue *value,
    SourceImageBounds *output) {
    size_t count = 0;
    SourceJsonStatus status = source_image_document_array(reader, value, sizeof(char *), &count);
    if (status != SOURCE_JSON_OK) { return status; }
    const char **paths = count ? source_arena_allocate(&reader->output->capture.arena, count * sizeof(*paths)) : NULL;
    if (count && !paths) { return source_image_document_error(reader, value, "Cannot retain observation boundaries", SOURCE_JSON_OUT_OF_MEMORY); }
    size_t i = 0;
    for (const SourceJsonValue *row = value->first; row; row = row->next, i++) {
        status = source_image_document_path(reader, row, false, &paths[i]);
        if (status != SOURCE_JSON_OK) { return status; }
    }
    *output = (SourceImageBounds){paths, count}; return SOURCE_JSON_OK;
}
