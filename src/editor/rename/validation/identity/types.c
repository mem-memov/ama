/** @file
 * @brief Pair concrete specialization owners by their complete finite nominal graphs.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) includes every capture;
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) requires an exact bijection.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "semantic/static/closure/plan.h"
/** A memoized vertex pair is expanded at most once, without recursive C calls.
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) requires finite exact traversal.
 */
typedef struct TermPair { struct TermPair *next; const StaticTerm *original; const StaticTerm *candidate; } TermPair;
/** Preserve module origins, never the order in which specializations were discovered.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) defines nominal origin identity.
 */
static bool module(const ProjectModule *original, const ProjectModule *candidate) {
    return original == NULL ? candidate == NULL : candidate != NULL
        && strcmp(original->source->logical_path,candidate->source->logical_path) == 0;
}
/** A formal retains its original owner, role, ordinal and exactly mapped defining site.
 * [TPL-02](../../../../../spec/language/templates.md#tpl-02) separates identical names in different scopes.
 */
static bool formal(const RenameIdentity *check, const StaticSymbol *original, const StaticSymbol *candidate) {
    return original && candidate && module(original->owner->definition,candidate->owner->definition)
        && original->role == candidate->role && original->ordinal == candidate->ordinal
        && rename_same_span(check,original->site,candidate->site)
        && rename_same_span(check,original->name,candidate->name);
}
/** Refuse allocation failure with a retained diagnostic; a repeated pair needs no further work.
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) forbids depth limits or guessed equality.
 */
static bool enqueue(RenameIdentity *check, TermPair **first, TermPair **last,
    const StaticTerm *original, const StaticTerm *candidate) {
    if (!original || !candidate) { return original == NULL && candidate == NULL; }
    for (const TermPair *pair = *first; pair; pair = pair->next) {
        if (pair->original == original && pair->candidate == candidate) { return true; }
    }
    TermPair *pair = source_arena_allocate(&check->plan->candidate.arena,sizeof(*pair));
    if (!pair) { return editor_rename_fail(check->plan,DIAGNOSTIC_OUT_OF_MEMORY,
        "Rename specialization correspondence allocation failed",check->plan->target.definition); }
    *pair = (TermPair){.original=original,.candidate=candidate};
    if (*last) { (*last)->next = pair; } else { *first = pair; }
    *last = pair;
    return true;
}
/** Nominal values retain the complete ordered capture/residual partition of original formals.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) includes otherwise unused captures.
 */
static bool terms(RenameIdentity *check, const StaticTerm *original, const StaticTerm *candidate) {
    TermPair *first = NULL, *last = NULL;
    if (!enqueue(check,&first,&last,original,candidate)) { return false; }
    for (const TermPair *pair = first; pair; pair = pair->next) {
        const StaticTerm *left = pair->original, *right = pair->candidate;
        if (left->kind != STATIC_TERM_NOMINAL || right->kind != STATIC_TERM_NOMINAL
            || left->category != right->category || left->ground != right->ground) { return false; }
        const StaticNominalTerm *a = &left->as.nominal, *b = &right->as.nominal;
        if (!module(a->origin->definition,b->origin->definition)
            || a->capture_count != b->capture_count || a->residual_count != b->residual_count) { return false; }
        const StaticTermCapture *capture = a->first_capture, *fresh = b->first_capture;
        while (capture && fresh) {
            if (!formal(check,capture->formal,fresh->formal)
                || !enqueue(check,&first,&last,capture->value,fresh->value)) { return false; }
            capture = capture->next;
            fresh = fresh->next;
        }
        if (capture || fresh) { return false; }
        const StaticTermResidual *residual = a->first_residual, *other = b->first_residual;
        while (residual && other) {
            if (!formal(check,residual->formal,other->formal)) { return false; }
            residual = residual->next;
            other = other->next;
        }
        if (residual || other) { return false; }
    }
    return true;
}
/** Borrow an accepted specialization's immutable concrete owner; raw ordinary owners have no applied map.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) keeps solved values separate from executable layout.
 */
static const StaticTerm *owner(const SemanticType *type) {
    return type->specialization ? type->specialization->context->registry_use->candidate->key->owner_type : NULL;
}
bool rename_identity_types(RenameIdentity *check) {
    size_t count = 0, fresh_count = 0;
    for (const SemanticType *type = check->plan->workspace->frontend.semantic.first; type; type = type->next) {
        if (count == SIZE_MAX/sizeof(*check->types)) { goto memory; }
        count++;
    }
    for (const SemanticType *type = check->plan->candidate.frontend.semantic.first; type; type = type->next) { fresh_count++; }
    if (count != fresh_count) { return false; }
    if (!count) { return true; }
    check->types = source_arena_allocate(&check->plan->candidate.arena,count*sizeof(*check->types));
    if (!check->types) { goto memory; }
    size_t index = 0;
    for (const SemanticType *old = check->plan->workspace->frontend.semantic.first; old; old = old->next) {
        const SemanticType *match = NULL;
        for (const SemanticType *fresh = check->plan->candidate.frontend.semantic.first; fresh; fresh = fresh->next) {
            if (!module(old->module,fresh->module)) { continue; }
            if (!terms(check,owner(old),owner(fresh))) {
                if (check->plan->diagnostic.message) { return false; }
                continue;
            }
            if (match) { return false; }
            match = fresh;
        }
        if (!match) { return false; }
        for (size_t earlier = 0; earlier < index; earlier++) {
            if (check->types[earlier].candidate == match) { return false; }
        }
        check->types[index++] = (RenameTypePair){old,match};
    }
    check->type_count = count;
    return true;
memory:
    return editor_rename_fail(check->plan,DIAGNOSTIC_OUT_OF_MEMORY,
        "Rename owner correspondence allocation failed",check->plan->target.definition);
}
bool rename_same_type(const RenameIdentity *check, const SemanticType *old, const SemanticType *fresh) {
    if (!old) { return fresh == NULL; }
    for (size_t index = 0; index < check->type_count; index++) {
        if (check->types[index].original == old) { return check->types[index].candidate == fresh; }
    }
    return false;
}
