/** @file
 * @brief Derive only required ancestor directories from exact final physical keys.
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * requires actual storage directories; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps original observations distinct from explicitly prepared scaffolding.
 */
#include "internal.h"
#include <string.h>
/** Choose the closest explicit root by exact component containment. */
static const ModuleRoot *containing_root(const SourceCopyImageBuilder *builder, const char *path) {
    const ModuleRoot *best = NULL;
    for (size_t i = 0; i < builder->original->root_count; i++) {
        const ModuleRoot *root = &builder->original->roots[i];
        if (source_image_below(path, root->path) && (!best || strlen(root->path) > strlen(best->path))) { best = root; }
    }
    return best;
}
SourceCopyImageStatus source_copy_image_parents(SourceCopyImageBuilder *builder) {
    for (size_t i = 0; i < builder->count; i++) {
        if (builder->changes[i].kind == SOURCE_IMAGE_REMOVE) { continue; }
        const char *path = builder->changes[i].entry.path;
        const ModuleRoot *root = containing_root(builder, path); builder->issue->path = path;
        if (!root) { return SOURCE_COPY_IMAGE_SCOPE; }
        const SourceCatalogImageEntry *root_entry = source_copy_image_original(builder, root->path);
        char *parent = source_arena_copy(builder->arena, path);
        if (!parent) { return SOURCE_COPY_IMAGE_MEMORY; }
        while (strcmp(parent, root->path)) {
            char *slash = strrchr(parent, '/');
            if (!slash) { return SOURCE_COPY_IMAGE_SCOPE; }
            if (slash == parent) { parent[1] = 0; } else { *slash = 0; }
            builder->issue->path = parent;
            SourceImageChange *change = source_copy_image_change(builder, parent);
            const SourceCatalogImageEntry *old = source_copy_image_original(builder, parent);
            if (change && change->kind != SOURCE_IMAGE_REMOVE) {
                if (!change->entry.metadata_captured || !S_ISDIR(change->entry.metadata.st_mode) || !change->entry.children_complete) { return SOURCE_COPY_IMAGE_COLLISION; }
                break;
            }
            if (old && !change) {
                if (!old->metadata_captured || !S_ISDIR(old->metadata.st_mode) || !old->children_complete) { return SOURCE_COPY_IMAGE_ORIGINAL; }
                break;
            }
            if (!strcmp(parent, root->path)) { return SOURCE_COPY_IMAGE_SCOPE; }
            struct stat metadata = root_entry->metadata;
            SourceCopyImageStatus status = source_copy_image_identity(builder, &metadata);
            if (status != SOURCE_COPY_IMAGE_OK) { return status; }
            SourceImageChange candidate = {old ? SOURCE_IMAGE_UPDATE : SOURCE_IMAGE_ADD,
                {.path = parent, .metadata = metadata, .metadata_captured = true, .children_complete = true}};
            if (change) { *change = candidate; }
            else {
                if (builder->count == builder->capacity) { return SOURCE_COPY_IMAGE_LIMIT; }
                builder->changes[builder->count++] = candidate;
            }
            /* Each added row borrows its own immutable key, never the next truncation. */
            parent = source_arena_copy(builder->arena, parent);
            if (!parent) { return SOURCE_COPY_IMAGE_MEMORY; }
        }
    }
    return SOURCE_COPY_IMAGE_OK;
}
