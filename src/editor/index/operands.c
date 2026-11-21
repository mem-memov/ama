/** @file
 * @brief Reuse checked access/source identities for references, indices and qualified constants.
 * [CONST-02](../../../spec/language/declarations.md#const-02) binds qualified constants exactly;
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) reuses the resolved graph.
 */
#include "../analysis/internal.h"
/** Index operands form a finite single-successor chain, so no recursive C walk is needed. Component contract: [CONST-02](../../../spec/language/declarations.md#const-02), [COMP-02](../../../spec/implementation/compiler.md#comp-02). */
bool editor_index_access(EditorAnalysis *analysis, const SemanticAccess *access) {
    for (; access != NULL; access = access->index) {
        if (!editor_index_add(analysis, (EditorOccurrence){.spelling=access->syntax->name,.symbol=access->symbol,.type=access->symbol->type})) { return false; }
    }
    return true;
}
/** Literal/self sources have no named target. Synthetic output transfers have no source syntax;
 * their actual callee label/caller receiver are indexed by the binding instead.
 * Qualified namespace and a genuinely written member keep exact original spans;
 * whole aliases and inherited components supply no member-name token here.
 * Component contract: [CONST-02](../../../spec/language/declarations.md#const-02), [COMP-02](../../../spec/implementation/compiler.md#comp-02). */
bool editor_index_source(EditorAnalysis *analysis, const SemanticSource *source) {
    if (source == NULL || source->syntax == NULL) { return true; }
    if (!editor_index_access(analysis, source->access)) { return false; }
    SourceSpan member = source->constant_member;
    SourceSpan operand = source->syntax->qualified_path;
    /* A whole alias or reduced inherited component writes no member token here.
     * Index only a genuine member within this operand, rather than making a
     * variable rename rewrite an alias key or another file's definition. */
    if (source->constant == NULL || member.source != operand.source
        || member.source == NULL || member.start < operand.start || member.end > operand.end) { return true; }
    return editor_index_add(analysis, (EditorOccurrence){.spelling=member,
        .symbol=source->constant,.type=source->constant->type});
}
