/** @file
 * @brief Retire rederived material evidence in reverse borrower order on every failed prefix.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps preparation scratch owned without retaining dormant alternative planners.
 */
#include "material.h"
#include <string.h>
void source_operation_material_init(SourceOperationMaterialReview *review, SourceAllocator allocator) {
    memset(review, 0, sizeof(*review)); source_arena_init(&review->arena, allocator);
    source_clone_map_init(&review->explicit_mains, allocator, NULL);
    source_clone_map_init(&review->expected_mains, allocator, NULL);
    source_copy_destinations_init(&review->destinations, allocator);
    source_file_map_init(&review->expected, allocator);
    source_copy_context_omissions_init(&review->omissions, allocator, NULL, NULL);
}
void source_operation_material_destroy(SourceOperationMaterialReview *review) {
    source_file_map_destroy(&review->expected); source_copy_context_omissions_destroy(&review->omissions);
    source_clone_map_destroy(&review->expected_mains);
    source_copy_destinations_destroy(&review->destinations); source_clone_map_destroy(&review->explicit_mains);
    source_arena_destroy(&review->arena); memset(review, 0, sizeof(*review));
}
