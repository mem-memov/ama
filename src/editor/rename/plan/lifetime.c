/** @file
 * @brief Own rename candidate and replacement storage without consuming the source workspace.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains all failed prefixes until teardown.
 */
#include "../internal.h"
/** Initialize every independent owner before any fallible preparation step. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
void editor_rename_init(EditorRename *plan, SourceAllocator allocator) {
    *plan = (EditorRename){0};
    source_arena_init(&plan->arena,allocator);
    source_manager_init(&plan->sources,allocator);
    editor_analysis_init(&plan->candidate,allocator);
}
/** Candidate diagnostics expire first; original workspace ownership always remains with the caller. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
void editor_rename_destroy(EditorRename *plan) {
    editor_analysis_destroy(&plan->candidate);
    source_arena_destroy(&plan->arena);
    source_manager_destroy(&plan->sources);
    *plan = (EditorRename){0};
}
/** Publish static prose with an exact borrowed original/name/candidate span, without allocating on failure. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool editor_rename_fail(EditorRename *plan, DiagnosticCode code, const char *message, SourceSpan span) {
    plan->diagnostic = (Diagnostic){.code=code,.message=message,.primary={.span=span}};
    return false;
}
