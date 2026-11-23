/** @file
 * @brief Pair finite original removal membership with exact observation and projection rows.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires explicit captured evidence rather than hidden removal recapture.
 */
#include "material.h"
#include <stdint.h>
SourceRemovalMaterialStatus source_removal_material_prepare(SourceArena *arena,
    const SourceRemovalSelection *selection, SourceRemovalMaterial *output) {
    if (!arena || !selection || !selection->valid || !output) { return SOURCE_REMOVAL_MATERIAL_INVALID; }
    size_t count = selection->path_count;
    if (count > SIZE_MAX / sizeof(SourceObservationRequest) || count > SIZE_MAX / sizeof(SourceImageChange)) {
        return SOURCE_REMOVAL_MATERIAL_MEMORY;
    }
    SourceObservationRequest *requests = count ? source_arena_allocate(arena, count * sizeof(*requests)) : NULL;
    SourceImageChange *changes = count ? source_arena_allocate(arena, count * sizeof(*changes)) : NULL;
    if (count && (!requests || !changes)) { return SOURCE_REMOVAL_MATERIAL_MEMORY; }
    size_t index = 0;
    for (const SourceRemovalSelectedFile *file = selection->selected; file; file = file->next) {
        if (index == count) { return SOURCE_REMOVAL_MATERIAL_INVALID; }
        requests[index] = (SourceObservationRequest){file->file->file.path,
            file->file->directory ? SOURCE_OBSERVE_CHILDREN : SOURCE_OBSERVE_CONTENT, false};
        changes[index++] = (SourceImageChange){SOURCE_IMAGE_REMOVE, {.path = file->file->file.path}};
    }
    if (index != count) { return SOURCE_REMOVAL_MATERIAL_INVALID; }
    *output = (SourceRemovalMaterial){requests, changes, count};
    return SOURCE_REMOVAL_MATERIAL_OK;
}
