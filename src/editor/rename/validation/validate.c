/** @file
 * @brief Admit source publication only after fresh version checks and exact checked-identity validation.
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06) forbids a plausible-name fallback;
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) keeps application explicitly versioned.
 */
#include "../internal.h"
/** Each check owns a fresh candidate; prior borrowed validation diagnostics expire before rebuilding it. Component contract: [ARCH-06](../../../../spec/implementation/compiler.md#arch-06), [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
bool editor_rename_validate(EditorRename *plan) {
    plan->validated = false;
    plan->diagnostic = (Diagnostic){0};
    if (!plan->prepared) {
        return editor_rename_fail(plan,DIAGNOSTIC_RENAME_CONFLICT,"Rename plan is not prepared",(SourceSpan){0});
    }
    editor_analysis_destroy(&plan->candidate);
    editor_analysis_init(&plan->candidate,plan->arena.allocator);
    plan->candidate.attempted = true;
    plan->source_index = NULL;
    plan->source_count = 0;
    if (!editor_workspace_configuration(&plan->candidate,plan->workspace->roots,plan->workspace->root_count,plan->workspace->entry)) {
        plan->diagnostic = plan->candidate.diagnostic;
        if (plan->diagnostic.message == NULL) {
            return editor_rename_fail(plan,DIAGNOSTIC_OUT_OF_MEMORY,"Rename candidate configuration allocation failed",plan->target.definition);
        }
        return false;
    }
    if (!editor_rename_checkpoint(plan) || !editor_rename_candidate(plan) || !editor_rename_identities(plan)) { return false; }
    plan->validated = true;
    return true;
}
/** The writer receives all files together only after a new validation; it owns atomic application under exclusive document authority. Component contract: [ARCH-06](../../../../spec/implementation/compiler.md#arch-06), [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
bool editor_rename_publish(EditorRename *plan, EditorRenameWriter writer, void *context) {
    if (!plan->prepared || writer == NULL) {
        return editor_rename_fail(plan,DIAGNOSTIC_RENAME_CONFLICT,"Rename publication requires a prepared plan and writer",(SourceSpan){0});
    }
    if (plan->first == NULL) { return true; }
    if (!editor_rename_validate(plan)) { return false; }
    if (!writer(context,plan->first)) {
        return editor_rename_fail(plan,DIAGNOSTIC_IO_FAILURE,"Rename document transaction was rejected",plan->target.definition);
    }
    return true;
}
