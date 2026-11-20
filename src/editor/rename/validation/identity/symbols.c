/** @file
 * @brief Prove declaration inventory bijection after ordered variable-name substitutions.
 * [NAME-06](../../../../../spec/language/declarations.md#name-06) retains local introduction versus reuse;
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) rejects silent captures.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Count the exact finite declaration domain with a checked allocation cardinality. Component contract: [NAME-06](../../../../../spec/language/declarations.md#name-06), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
static bool count_symbols(const SemanticProgram *program, size_t *output) {
    size_t count = 0;
    for (const SemanticType *type = program->first; type != NULL; type = type->next) {
        const SemanticSymbol *lists[] = {type->first,type->locals};
        for (size_t list = 0; list < 2; list++) {
            for (const SemanticSymbol *symbol = lists[list]; symbol != NULL; symbol = symbol->next) {
                if (count == SIZE_MAX/sizeof(RenameSymbolPair)) { return false; }
                count++;
            }
        }
    }
    *output = count;
    return true;
}
/** A renamed declaration changes only its complete base name; all other names and scope markers retain exact bytes/coordinates. Component contract: [NAME-06](../../../../../spec/language/declarations.md#name-06), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
static bool declaration(RenameIdentity *check, const SemanticSymbol *old, const SemanticSymbol *fresh) {
    if (!rename_same_span(check,old->name,fresh->name) || !rename_same_type(check,old->owner,fresh->owner)
        || !rename_same_type(check,old->type,fresh->type) || old->role != fresh->role || old->ordinal != fresh->ordinal
        || old->role_ordinal != fresh->role_ordinal || old->reference != fresh->reference || old->list != fresh->list
        || old->local != fresh->local || old->inferred != fresh->inferred) { return false; }
    SourceSpan expected = check->plan->target.original && old->original == check->plan->target.original ? check->plan->name : old->name;
    if (expected.end-expected.start != fresh->name.end-fresh->name.start
        || memcmp(source_bytes(expected.source)+expected.start,source_bytes(fresh->name.source)+fresh->name.start,expected.end-expected.start) != 0) { return false; }
    if (old->local && (!rename_same_span(check,old->scope->as.traversal.body_marker,fresh->scope->as.traversal.body_marker)
        || !rename_same_span(check,old->scope->as.traversal.closing_marker,fresh->scope->as.traversal.closing_marker))) { return false; }
    return true;
}
/** Construct one injective pairing; missing/added declarations or changed binding scopes are conflicts, never a valid rename. Component contract: [NAME-06](../../../../../spec/language/declarations.md#name-06), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
bool rename_identity_symbols(RenameIdentity *check) {
    size_t count = 0, candidate_count = 0;
    if (!count_symbols(&check->plan->workspace->frontend.semantic,&count)
        || !count_symbols(&check->plan->candidate.frontend.semantic,&candidate_count)) { goto memory; }
    if (count != candidate_count) { return false; }
    check->symbols = source_arena_allocate(&check->plan->candidate.arena,count*sizeof(*check->symbols));
    const SemanticSymbol **fresh = source_arena_allocate(&check->plan->candidate.arena,count*sizeof(*fresh));
    if (check->symbols == NULL || fresh == NULL) { goto memory; }
    size_t index = 0;
    for (const SemanticType *type = check->plan->workspace->frontend.semantic.first; type; type = type->next) {
        const SemanticSymbol *lists[] = {type->first,type->locals};
        for (size_t list = 0; list < 2; list++) {
            for (const SemanticSymbol *symbol = lists[list]; symbol; symbol = symbol->next) {
                check->symbols[index++] = (RenameSymbolPair){.original=symbol};
            }
        }
    }
    index = 0;
    for (const SemanticType *type = check->plan->candidate.frontend.semantic.first; type; type = type->next) {
        const SemanticSymbol *lists[] = {type->first,type->locals};
        for (size_t list = 0; list < 2; list++) {
            for (const SemanticSymbol *symbol = lists[list]; symbol; symbol = symbol->next) { fresh[index++] = symbol; }
        }
    }
    check->count = count;
    for (index = 0; index < count; index++) {
        const SemanticSymbol *match = NULL;
        for (size_t candidate = 0; candidate < count; candidate++) {
            if (declaration(check,check->symbols[index].original,fresh[candidate])) {
                if (match) { return false; }
                match = fresh[candidate];
            }
        }
        if (!match) { return false; }
        for (size_t earlier = 0; earlier < index; earlier++) {
            if (check->symbols[earlier].candidate == match) { return false; }
        }
        check->symbols[index].candidate = match;
    }
    return true;
memory:
    return editor_rename_fail(check->plan,DIAGNOSTIC_OUT_OF_MEMORY,"Rename identity correspondence allocation failed",check->plan->target.definition);
}
/** A nullable reference can match only its recorded declaration correspondence, not a same-spelled candidate. Component contract: [NAME-06](../../../../../spec/language/declarations.md#name-06), [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06). */
bool rename_same_symbol(const RenameIdentity *check, const SemanticSymbol *old, const SemanticSymbol *fresh) {
    if (old == NULL) { return fresh == NULL; }
    for (size_t index = 0; index < check->count; index++) {
        if (check->symbols[index].original == old) { return check->symbols[index].candidate == fresh; }
    }
    return false;
}
