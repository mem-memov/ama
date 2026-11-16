/** @file
 * @brief Destroy editor indexes before their borrowed frontend/AST/source owners.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) requires explicit reverse lifetime.
 */
#include "analysis.h"
/** No owner observes filesystem or executes native code during initialization. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
void editor_analysis_init(EditorAnalysis *analysis, SourceAllocator allocator) {
    *analysis = (EditorAnalysis){.state=EDITOR_UNAVAILABLE};
    compiler_frontend_init(&analysis->frontend, allocator);
    source_arena_init(&analysis->arena, allocator);
    ast_module_init(&analysis->inspection, allocator);
}
/** Failed prefixes have the same destruction path as a complete immutable snapshot. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
void editor_analysis_destroy(EditorAnalysis *analysis) {
    source_arena_destroy(&analysis->arena);
    ast_module_destroy(&analysis->inspection);
    compiler_frontend_destroy(&analysis->frontend);
    *analysis = (EditorAnalysis){0};
}
