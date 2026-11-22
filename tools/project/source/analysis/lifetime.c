/** @file
 * @brief Retire cached proof owners before original source graph identity.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) preserves exact
 * reverse ownership even when original/probe collection fails.
 */
#include <string.h>
#include "analysis.h"
void source_analysis_init(SourceAnalysis *analysis, SourceAllocator allocator) {
    memset(analysis, 0, sizeof(*analysis)); source_arena_init(&analysis->arena, allocator);
    source_compiler_client_init(&analysis->original, allocator);
}
void source_analysis_destroy(SourceAnalysis *analysis) {
    for (SourceAnalysisProbe *probe = analysis->probes; probe; probe = probe->next) {
        source_compiler_client_destroy(&probe->compiler);
    }
    source_compiler_client_destroy(&analysis->original); source_arena_destroy(&analysis->arena);
    memset(analysis, 0, sizeof(*analysis));
}
