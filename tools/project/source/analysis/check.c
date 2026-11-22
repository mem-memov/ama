/** @file
 * @brief Admit the complete original compiler reply before exposing any provider fact.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires original checked bindings;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed copied names and child data.
 */
#include "analysis.h"
SourceAnalysisStatus source_analysis_check(SourceAnalysis *analysis, const DeliverySourceSnapshotView *view,
    const char *compiler, const char *request_path) {
    if (!analysis || analysis->attempted || !view || !compiler || !*compiler || !request_path || !*request_path) {
        return SOURCE_ANALYSIS_INVALID;
    }
    analysis->attempted = true; analysis->view = *view;
    analysis->compiler = source_arena_copy(&analysis->arena, compiler);
    analysis->request_path = source_arena_copy(&analysis->arena, request_path);
    if (!analysis->compiler || !analysis->request_path) { return analysis->status = SOURCE_ANALYSIS_MEMORY; }
    if (source_compiler_client_run(&analysis->original, analysis->compiler, analysis->request_path, &analysis->view, NULL) != SOURCE_COMPILER_OK) {
        return analysis->status = SOURCE_ANALYSIS_COMPILER;
    }
    analysis->valid = true; return analysis->status = SOURCE_ANALYSIS_OK;
}
