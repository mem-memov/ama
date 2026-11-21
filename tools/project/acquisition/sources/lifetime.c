/** @file
 * @brief Own acquisition metadata independently of compiler or transport process state.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains all
 * failed scratch until one destructor; [PKG-06](../../../../spec/project/packages.md#pkg-06)
 * limits source-key sharing to this operation.
 */
#include "internal.h"
void acquisition_sources_init(AcquisitionSources *sources, SourceAllocator allocator, const ModulePackage *prepared) {
    *sources = (AcquisitionSources){.prepared = prepared};
    source_arena_init(&sources->arena, allocator);
}
void acquisition_sources_destroy(AcquisitionSources *sources) {
    source_arena_destroy(&sources->arena);
    *sources = (AcquisitionSources){0};
}
