/** @file
 * @brief Verify whole-program declaration initialization, statement graphs and exact source occurrences after a rename.
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) makes preservation explicit;
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) keeps qualified constant identity exact.
 */
#include "internal.h"
/** Each indexed spelling must keep its declaration/type correspondence, including unchanged names in affected scopes. Component contract: [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06), [CONST-02](../../../../../spec/language/declarations.md#const-02). */
bool rename_identity_occurrences(RenameIdentity *check) {
    const EditorOccurrence *old = check->plan->workspace->first, *fresh = check->plan->candidate.first;
    while (old != NULL && fresh != NULL) {
        if (!rename_same_span(check,old->spelling,fresh->spelling) || old->declaration != fresh->declaration
            || !rename_same_symbol(check,old->symbol,fresh->symbol) || !rename_same_type(old->type,fresh->type)) { return false; }
        old = old->next;
        fresh = fresh->next;
    }
    return old == NULL && fresh == NULL;
}
/** Original constructor/literal/continuation transfers retain the same sources and execution-independent initialization order. Component contract: [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06), [CONST-02](../../../../../spec/language/declarations.md#const-02). */
static bool initializers(const RenameIdentity *check, const SemanticSymbol *old, const SemanticSymbol *fresh) {
    if (old->initializer_count != fresh->initializer_count) { return false; }
    const SemanticInitializer *a = old->initializers, *b = fresh->initializers;
    while (a != NULL && b != NULL) {
        if (a->transfer != b->transfer || !rename_same_source(check,a->source,b->source)) { return false; }
        a = a->next;
        b = b->next;
    }
    return a == NULL && b == NULL;
}
/** All module identities and complete flat statement inventories must agree; no unreferenced type escapes checking. Component contract: [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06), [CONST-02](../../../../../spec/language/declarations.md#const-02). */
bool rename_identity_program(RenameIdentity *check) {
    size_t old_count = 0, fresh_count = 0;
    for (const SemanticType *type = check->plan->candidate.frontend.semantic.first; type != NULL; type = type->next) { fresh_count++; }
    for (const SemanticType *old = check->plan->workspace->frontend.semantic.first; old != NULL; old = old->next) {
        old_count++;
        const SemanticType *fresh = semantic_find_type(&check->plan->candidate.frontend.semantic,old->module->source->logical_path);
        if (fresh == NULL || old->executable != fresh->executable || old->cloneable != fresh->cloneable
            || old->requires_size_bridge != fresh->requires_size_bridge || old->symbol_count != fresh->symbol_count
            || old->module->syntax.sections.kind != fresh->module->syntax.sections.kind
            || !rename_same_type(old->clone_seed,fresh->clone_seed)
            || !rename_same_symbol(check,old->clone_cause,fresh->clone_cause)) { return false; }
        for (size_t role = 0; role < 4; role++) { if (old->counts[role] != fresh->counts[role]) { return false; } }
        const SemanticStatement *a = old->statements, *b = fresh->statements;
        while (a != NULL && b != NULL) {
            if (!rename_same_statement(check,a,b)) { return false; }
            a = a->next;
            b = b->next;
        }
        if (a != NULL || b != NULL) { return false; }
    }
    if (old_count != fresh_count) { return false; }
    for (size_t index = 0; index < check->count; index++) {
        if (!initializers(check,check->symbols[index].original,check->symbols[index].candidate)) { return false; }
    }
    const SemanticConstantPlan *a = check->plan->workspace->frontend.semantic.constant_first;
    const SemanticConstantPlan *b = check->plan->candidate.frontend.semantic.constant_first;
    while (a != NULL && b != NULL) {
        if (a->ordinal != b->ordinal || !rename_same_symbol(check,a->symbol,b->symbol)) { return false; }
        a = a->next;
        b = b->next;
    }
    return a == NULL && b == NULL;
}
/** Compilation is necessary but insufficient: require a complete declaration/occurrence/control graph isomorphism. Component contract: [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06), [CONST-02](../../../../../spec/language/declarations.md#const-02). */
bool editor_rename_identities(EditorRename *plan) {
    RenameIdentity check = {.plan=plan};
    if (rename_identity_symbols(&check) && rename_identity_occurrences(&check) && rename_identity_program(&check)) { return true; }
    return plan->diagnostic.message != NULL ? false : editor_rename_fail(plan,DIAGNOSTIC_RENAME_CONFLICT,
        "Rename would change a checked declaration, reference or operation identity",plan->target->name);
}
