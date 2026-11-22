/** @file
 * @brief Retire source-copy closure metadata in reverse borrowed-fact order.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps complete and refused preparation under one explicit operation owner.
 */
#include "closure.h"
void source_copy_closure_init(SourceCopyClosure *copy, SourceAllocator allocator,
    const SourceRootCapture *capture, SourceAnalysis *analysis, const SourceDeclaredGroup *declared) {
    *copy = (SourceCopyClosure){0}; source_arena_init(&copy->arena, allocator);
    source_expansion_init(&copy->expansion, allocator, capture, analysis, declared);
    acquisition_closure_init(&copy->closure, allocator);
}
void source_copy_closure_destroy(SourceCopyClosure *copy) {
    acquisition_closure_destroy(&copy->closure); source_expansion_destroy(&copy->expansion);
    source_arena_destroy(&copy->arena); *copy = (SourceCopyClosure){0};
}
