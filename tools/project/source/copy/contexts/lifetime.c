/** @file
 * @brief Own context-omission obligations without owning source or final material.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps failed preparation destructible and separate from publication authority.
 */
#include "omissions.h"
#include <string.h>
void source_copy_context_omissions_init(SourceCopyContextOmissions *owner, SourceAllocator allocator,
    const SourceCopyClosure *copy, const SourceCloneMap *mains) {
    memset(owner, 0, sizeof(*owner));
    source_arena_init(&owner->arena, allocator);
    owner->copy = copy;
    owner->mains = mains;
}
void source_copy_context_omissions_destroy(SourceCopyContextOmissions *owner) {
    source_arena_destroy(&owner->arena);
}
