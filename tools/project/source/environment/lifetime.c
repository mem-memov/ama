/** @file
 * @brief Retire partial package documents before shared environment metadata.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps failed preparation owned without publishing source changes.
 */
#include "environment.h"
#include <string.h>
void source_package_environment_init(SourcePackageEnvironment *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
}
void source_package_environment_destroy(SourcePackageEnvironment *owner) {
    for (SourcePackageScope *row = owner->first; row; row = row->next) { configuration_document_destroy(&row->document); }
    source_arena_destroy(&owner->arena);
}
