/** @file
 * @brief Own physical observation scratch separately from the immutable original capture.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains complete
 * and failed prefixes; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * prohibits replacing an older original capture after contradiction.
 */
#include "internal.h"
#include <string.h>
void source_observed_image_init(SourceObservedImage *observed, SourceAllocator allocator) {
    memset(observed, 0, sizeof(*observed));
    source_arena_init(&observed->arena, allocator); source_manager_init(&observed->files, allocator);
    module_catalog_init(&observed->physical, allocator); module_catalog_init(&observed->catalog, allocator);
    observed->reader = (ModuleCatalogReader){.tree = {.context = observed, .root = source_observation_root,
        .child = source_observation_child, .entries = source_observation_entries}, .read = source_observation_read};
}
void source_observed_image_destroy(SourceObservedImage *observed) {
    module_catalog_destroy(&observed->catalog); module_catalog_destroy(&observed->physical);
    source_manager_destroy(&observed->files); source_arena_destroy(&observed->arena);
    observed->first = observed->last = NULL; observed->image = (SourceCatalogImage){0};
    observed->valid = false;
}
