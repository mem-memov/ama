/** @file
 * @brief Bind every mapped file/directory to its observed original and replacement authority.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains opaque bytes; [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * requires explicit independently checked removal sets before replacement.
 */
#include "internal.h"
#include <string.h>
SourceCopyImageStatus source_copy_image_members(SourceCopyImageBuilder *builder, const SourceCopyMaterial *material) {
    for (size_t i = 0; i < material->count; i++) {
        const SourceCopyMaterialEntry *member = &material->entries[i]; const SourceFileMapping *map = member->mapping;
        builder->issue->mapping = map; builder->issue->path = map->source;
        const SourceCatalogImageEntry *source = source_copy_image_original(builder, map->source);
        bool directory = map->original->directory;
        if (!source || !source->metadata_captured || (directory ? !S_ISDIR(source->metadata.st_mode) || !source->children_complete
            : !S_ISREG(source->metadata.st_mode) || !source->content
                || source_length(source->content) != source_length(map->original->file.content)
                || memcmp(source_bytes(source->content), source_bytes(map->original->file.content), source_length(source->content)))) {
            return SOURCE_COPY_IMAGE_ORIGINAL;
        }
        builder->issue->path = map->destination;
        const SourceCatalogImageEntry *old = source_copy_image_original(builder, map->destination);
        SourceImageChange *change = source_copy_image_change(builder, map->destination);
        if ((change && (change->kind != SOURCE_IMAGE_REMOVE || !old)) || (old && !change)) { return SOURCE_COPY_IMAGE_COLLISION; }
        struct stat metadata = source->metadata;
        SourceCopyImageStatus status = source_copy_image_identity(builder, &metadata);
        if (status != SOURCE_COPY_IMAGE_OK) { return status; }
        SourceImageChange candidate = {old ? SOURCE_IMAGE_UPDATE : SOURCE_IMAGE_ADD,
            {.path = map->destination, .metadata = metadata, .content = member->content,
                .metadata_captured = true, .children_complete = directory}};
        if (change) { *change = candidate; }
        else {
            if (builder->count == builder->capacity) { return SOURCE_COPY_IMAGE_LIMIT; }
            builder->changes[builder->count++] = candidate;
        }
    }
    return SOURCE_COPY_IMAGE_OK;
}
