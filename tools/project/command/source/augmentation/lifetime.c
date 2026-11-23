/** @file
 * @brief Retire augmented raw observations before their catalog and descriptor owners.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * owns failed partial original preparation without replacing configured facts.
 */
#include "augmentation.h"
#include <string.h>
void project_source_augmentation_init(ProjectSourceAugmentation *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
    source_root_capture_init(&owner->capture, allocator); source_tree_selection_init(&owner->source, allocator);
}
void project_source_augmentation_destroy(ProjectSourceAugmentation *owner) {
    source_tree_selection_destroy(&owner->source); source_root_capture_destroy(&owner->capture);
    source_arena_destroy(&owner->arena); memset(owner, 0, sizeof(*owner));
}
