/** @file
 * @brief Compare resolved operand graphs through exact declaration and source-coordinate mappings.
 * [COMP-02](../../../../../spec/implementation/compiler.md#comp-02) provides one checked IR;
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) requires explicit preservation invariants.
 */
#include <string.h>
#include "internal.h"
/** Original half-open boundaries map exactly into the candidate's retained source. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
bool rename_same_span(const RenameIdentity *check, SourceSpan old, SourceSpan fresh) {
    SourceSpan mapped;
    return editor_rename_span(check->plan,old,&mapped) && mapped.source == fresh.source
        && mapped.start == fresh.start && mapped.end == fresh.end;
}
/** Nested list indices retain each exact caller declaration and storage interpretation. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
bool rename_same_access(const RenameIdentity *check, const SemanticAccess *old, const SemanticAccess *fresh) {
    while (old != NULL && fresh != NULL) {
        if (!rename_same_symbol(check,old->symbol,fresh->symbol) || old->reference != fresh->reference || old->list != fresh->list
            || !rename_same_span(check,old->syntax->span,fresh->syntax->span)
            || !rename_same_span(check,old->syntax->name,fresh->syntax->name)) { return false; }
        old = old->index;
        fresh = fresh->index;
    }
    return old == NULL && fresh == NULL;
}
/** Synthetic output transfers retain no fabricated AST; source mode/type/ownership,
 * selected declaration and complete written operands remain exact. Actual written
 * member tokens are checked by the occurrence identity pass. Alias-owned optional
 * provenance is not an edited module-source interval and must not be mapped as one.
 * Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
bool rename_same_source(const RenameIdentity *check, const SemanticSource *old, const SemanticSource *fresh) {
    if (old == NULL) { return fresh == NULL; }
    if (fresh == NULL || old->kind != fresh->kind || old->reference != fresh->reference || old->list != fresh->list
        || !rename_same_type(check,old->type,fresh->type) || !rename_same_symbol(check,old->constant,fresh->constant)
        || !rename_same_symbol(check,old->declaration,fresh->declaration) || !rename_same_span(check,old->site,fresh->site)
        || !rename_same_access(check,old->access,fresh->access)) { return false; }
    if (old->syntax == NULL) { return fresh->syntax == NULL; }
    return fresh->syntax != NULL && old->syntax->kind == fresh->syntax->kind
        && old->syntax->constant_list == fresh->syntax->constant_list
        && rename_same_span(check,old->syntax->span,fresh->syntax->span)
        && rename_same_span(check,old->syntax->qualified_path,fresh->syntax->qualified_path)
        && rename_same_span(check,old->syntax->payload,fresh->syntax->payload);
}
/** Following an alias compares its unique owned syntax identity; control cycles need no recursive traversal. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
bool rename_same_link(const RenameIdentity *check, const SemanticStatement *old, const SemanticStatement *fresh) {
    return old == NULL ? fresh == NULL : fresh != NULL && old->syntax->kind == fresh->syntax->kind
        && rename_same_span(check,old->syntax->span,fresh->syntax->span);
}
