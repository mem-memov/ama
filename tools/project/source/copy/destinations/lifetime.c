/** @file
 * @brief Own logical copy-destination preparation without owning original source facts.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps complete
 * and failed scratch operation-local; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps prepared candidate names separate from publication authority.
 */
#include "destinations.h"
#include <string.h>
void source_copy_destinations_init(SourceCopyDestinations *destinations, SourceAllocator allocator) {
    memset(destinations, 0, sizeof(*destinations));
    source_arena_init(&destinations->arena, allocator);
}
void source_copy_destinations_destroy(SourceCopyDestinations *destinations) {
    source_arena_destroy(&destinations->arena);
    memset(destinations, 0, sizeof(*destinations));
}
