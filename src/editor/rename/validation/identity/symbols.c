/** @file
 * @brief Prove declaration inventory bijection after ordered variable-name substitutions.
 * [NAME-06](../../../../../spec/language/declarations.md#name-06) retains local introduction versus reuse;
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) rejects silent captures.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
/** Complete owner identity plus unique declaration byte range orders symbols independently of their names. */
static int key(const SemanticSymbol *left, const SemanticSymbol *right) {
    int owner = strcmp(left->owner->module->source->logical_path,right->owner->module->source->logical_path);
    if (owner != 0) { return owner; }
    if (left->name.start != right->name.start) { return left->name.start < right->name.start ? -1 : 1; }
    return (left->name.end > right->name.end)-(left->name.end < right->name.end);
}
/** Sort original-key pairs before binary correspondence lookup. */
static int original_order(const void *left, const void *right) {
    return key(((const RenameSymbolPair *)left)->original,((const RenameSymbolPair *)right)->original);
}
/** Candidate declaration order remains monotone under disjoint nonempty name substitutions. */
static int candidate_order(const void *left, const void *right) {
    return key(*(const SemanticSymbol *const *)left,*(const SemanticSymbol *const *)right);
}
/** Count the exact finite declaration domain with a checked allocation cardinality. */
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
/** A renamed declaration changes only its complete base name; all other names and scope markers retain exact bytes/coordinates. */
static bool declaration(RenameIdentity *check, const SemanticSymbol *old, const SemanticSymbol *fresh) {
    if (!rename_same_span(check,old->name,fresh->name) || !rename_same_type(old->owner,fresh->owner)
        || !rename_same_type(old->type,fresh->type) || old->role != fresh->role || old->ordinal != fresh->ordinal
        || old->role_ordinal != fresh->role_ordinal || old->reference != fresh->reference || old->list != fresh->list
        || old->local != fresh->local || old->inferred != fresh->inferred) { return false; }
    SourceSpan expected = old == check->plan->target ? check->plan->name : old->name;
    if (expected.end-expected.start != fresh->name.end-fresh->name.start
        || memcmp(source_bytes(expected.source)+expected.start,source_bytes(fresh->name.source)+fresh->name.start,expected.end-expected.start) != 0) { return false; }
    if (old->local && (!rename_same_span(check,old->scope->as.traversal.body_marker,fresh->scope->as.traversal.body_marker)
        || !rename_same_span(check,old->scope->as.traversal.closing_marker,fresh->scope->as.traversal.closing_marker))) { return false; }
    return true;
}
/** Construct one sorted pairing; missing/added declarations or changed binding scopes are conflicts, never a valid rename. */
bool rename_identity_symbols(RenameIdentity *check) {
    size_t count = 0, candidate_count = 0;
    if (!count_symbols(&check->plan->workspace->frontend.semantic,&count)
        || !count_symbols(&check->plan->candidate.frontend.semantic,&candidate_count)) { goto memory; }
    if (count != candidate_count) { return false; }
    check->symbols = source_arena_allocate(&check->plan->candidate.arena,count*sizeof(*check->symbols));
    const SemanticSymbol **fresh = source_arena_allocate(&check->plan->candidate.arena,count*sizeof(*fresh));
    if (check->symbols == NULL || fresh == NULL) { goto memory; }
    size_t index = 0;
    for (const SemanticType *type = check->plan->workspace->frontend.semantic.first; type != NULL; type = type->next) {
        const SemanticSymbol *lists[] = {type->first,type->locals};
        for (size_t list = 0; list < 2; list++) {
            for (const SemanticSymbol *symbol = lists[list]; symbol != NULL; symbol = symbol->next) {
                check->symbols[index++] = (RenameSymbolPair){.original=symbol};
            }
        }
    }
    index = 0;
    for (const SemanticType *type = check->plan->candidate.frontend.semantic.first; type != NULL; type = type->next) {
        const SemanticSymbol *lists[] = {type->first,type->locals};
        for (size_t list = 0; list < 2; list++) {
            for (const SemanticSymbol *symbol = lists[list]; symbol != NULL; symbol = symbol->next) { fresh[index++] = symbol; }
        }
    }
    qsort(check->symbols,count,sizeof(*check->symbols),original_order);
    qsort(fresh,count,sizeof(*fresh),candidate_order);
    check->count = count;
    for (index = 0; index < count; index++) {
        check->symbols[index].candidate = fresh[index];
        if (!declaration(check,check->symbols[index].original,fresh[index])) { return false; }
    }
    return true;
memory:
    return editor_rename_fail(check->plan,DIAGNOSTIC_OUT_OF_MEMORY,"Rename identity correspondence allocation failed",check->plan->target->name);
}
/** A nullable reference can match only its recorded declaration correspondence, not a same-spelled candidate. */
bool rename_same_symbol(const RenameIdentity *check, const SemanticSymbol *old, const SemanticSymbol *fresh) {
    if (old == NULL) { return fresh == NULL; }
    size_t low = 0, high = check->count;
    while (low < high) {
        size_t middle = low+(high-low)/2;
        int order = key(old,check->symbols[middle].original);
        if (order == 0) { return old == check->symbols[middle].original && fresh == check->symbols[middle].candidate; }
        if (order < 0) { high = middle; } else { low = middle+1; }
    }
    return false;
}
