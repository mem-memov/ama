/** @file
 * @brief Own command seed candidates in borrower-before-owner teardown order.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * retains failed metadata until destruction without changing original inputs.
 */
#include "seeds.h"
void project_source_seeds_init(ProjectSourceSeeds *owner, SourceAllocator allocator) {
    *owner = (ProjectSourceSeeds){0}; source_arena_init(&owner->arena, allocator);
    source_copy_closure_init(&owner->copy, allocator, NULL, NULL, NULL);
    source_clone_map_init(&owner->explicit_mains, allocator, NULL);
    source_copy_destinations_init(&owner->destinations, allocator);
}
void project_source_seeds_destroy(ProjectSourceSeeds *owner) {
    source_copy_destinations_destroy(&owner->destinations);
    source_clone_map_destroy(&owner->explicit_mains);
    source_copy_closure_destroy(&owner->copy);
    source_arena_destroy(&owner->arena); *owner = (ProjectSourceSeeds){0};
}
