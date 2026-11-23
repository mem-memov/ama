/** @file
 * @brief Derive destination storage from actual selected final package versions.
 * [PKG-02](../../../../../spec/project/packages.md#pkg-02)
 * supplies one selected physical copy per complete version identity;
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids choosing a physical owner from a basename or first matching prefix.
 */
#include "internal.h"
#include <stdint.h>
SourceOperationMaterialStatus source_operation_material_versions(SourceOperationMaterialReview *review) {
    if (!review->plan->inputs.copy) { return SOURCE_OPERATION_MATERIAL_OK; }
    size_t count = 0;
    for (const SourcePackageScope *scope = review->plan->environment.first; scope; scope = scope->next) {
        if (count == SIZE_MAX) { return SOURCE_OPERATION_MATERIAL_MEMORY; } count++;
    }
    if (count > SIZE_MAX / sizeof(SourceVersionPlacement)) { return SOURCE_OPERATION_MATERIAL_MEMORY; }
    SourceVersionPlacement *versions = count ? source_arena_allocate(&review->arena, count * sizeof(*versions)) : NULL;
    if (count && !versions) { return SOURCE_OPERATION_MATERIAL_MEMORY; }
    size_t index = 0;
    for (const SourcePackageScope *scope = review->plan->environment.first; scope; scope = scope->next) {
        versions[index++] = (SourceVersionPlacement){scope->version->logical_path, scope->version->physical_path};
    }
    review->versions = versions; review->version_count = count;
    return SOURCE_OPERATION_MATERIAL_OK;
}
