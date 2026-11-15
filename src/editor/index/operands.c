/** @file
 * @brief Reuse checked access/source identities for references, indices and qualified constants.
 * [CONST-02](../../../spec/language/declarations.md#const-02) binds qualified constants exactly;
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) reuses the resolved graph.
 */
#include "../analysis/internal.h"
/** Index operands form a finite single-successor chain, so no recursive C walk is needed. */
bool editor_index_access(EditorAnalysis *analysis, const SemanticAccess *access) {
    for (; access != NULL; access = access->index) {
        if (!editor_index_add(analysis, access->syntax->name, access->symbol, access->symbol->type, false)) { return false; }
    }
    return true;
}
/** Literal/self sources have no named target. Synthetic output transfers have no source syntax;
 * their actual callee label/caller receiver are indexed by the binding instead.
 * Qualified namespace and member keep distinct exact spans.
 */
bool editor_index_source(EditorAnalysis *analysis, const SemanticSource *source) {
    if (source == NULL || source->syntax == NULL) { return true; }
    if (!editor_index_access(analysis, source->access)) { return false; }
    if (source->syntax->module_path.source != NULL) {
        const SemanticType *owner = semantic_type(&analysis->frontend.semantic, source->syntax->module_path);
        if (!editor_index_add(analysis, source->syntax->module_path, NULL, owner, false)) { return false; }
    }
    return source->constant == NULL || editor_index_add(analysis, source->syntax->member,
        source->constant, source->constant->type, false);
}
