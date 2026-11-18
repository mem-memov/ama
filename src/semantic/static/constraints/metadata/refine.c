/** @file
 * @brief Admit only complete independent metadata facts preserving every known declaration.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) fixes nested named signatures.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids conflicting refinement.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../signature/signature.h"

/** Comparison never mutates either accepted source metadata or prepared facts.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) checks whole recursive sets.
 */
static bool compare(StaticConstraintSolver *solver, const StaticSignature *left,
    const StaticSignature *right, SourceSpan site, StaticSignatureComparison *output) {
    return static_constraint_signature(solver, left, right, site, output);
}

/** Prepared facts must be fully known: accepting Pending here would hide an
 * unresolved dependency instead of adding information.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids guessed evidence.
 */
static bool complete(StaticConstraintSolver *solver, StaticCategory category,
    const StaticSignature *signature, SourceSpan site) {
    if (category == STATIC_CATEGORY_TYPE && signature == NULL) { return true; }
    if (category == STATIC_CATEGORY_TEMPLATE && signature != NULL && signature->count != 0) {
        StaticSignatureComparison result;
        if (!compare(solver, signature, signature, site, &result)) { return false; }
        if (result == STATIC_SIGNATURE_EQUAL) { return true; }
    }
    return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, site,
        "Prepared static metadata must be a fully resolved Type or Template description", NULL, NULL);
}

/** A fact can fill unknown fields, never change already known names/categories.
 * Equal complete facts require no extra publication; source storage remains
 * untouched. [TPL-10](../../../../../spec/language/templates.md#tpl-10).
 */
bool static_constraint_describe(StaticConstraintSolver *solver, const StaticTerm *term,
    StaticCategory category, const StaticSignature *signature, SourceSpan site) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (!static_term_has_value(solver->terms, term)) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, site,
            "Static metadata target must belong to its term owner", NULL, NULL);
    }
    for (const StaticConstraintMetadata *fact = solver->first_metadata; fact; fact = fact->next) {
        if (fact->term == term && fact->category == category && fact->signature == signature) { return true; }
    }
    if (!complete(solver, category, signature, site)) { return false; }
    StaticCategory previous;
    const StaticSignature *previous_signature;
    static_constraint_description(solver, term, &previous, &previous_signature);
    if (previous != STATIC_CATEGORY_DEFERRED) {
        if (previous != category) {
            return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, site,
                "Static metadata contradicts a known category", term, NULL);
        }
        if (category == STATIC_CATEGORY_TYPE) { return true; }
        StaticSignatureComparison result;
        if (!compare(solver, previous_signature, signature, site, &result)) { return false; }
        if (result == STATIC_SIGNATURE_DIFFERENT) {
            return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, site,
                "Static metadata contradicts a known constructor signature", term, NULL);
        }
        if (result == STATIC_SIGNATURE_EQUAL) { return true; }
    }
    if (solver->metadata_count == SIZE_MAX) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, site,
            "Static metadata count exceeds representable storage", term, NULL);
    }
    StaticConstraintMetadata *fact = static_constraint_allocate(solver, sizeof(*fact));
    if (fact == NULL) { return false; }
    *fact = (StaticConstraintMetadata){.term = term, .category = category, .signature = signature,
        .site = site, .next = solver->first_metadata};
    solver->first_metadata = fact;
    solver->metadata_count++;
    solver->dirty = true;
    return true;
}
