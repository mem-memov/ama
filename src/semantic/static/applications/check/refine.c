/** @file
 * @brief Admit complete independent expected-signature facts while preserving known source constraints.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) requires exact recursive signatures.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves scoped application context.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial facts.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../internal.h"

/** Exact owner publication precedes any source/candidate dereference.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves use identity.
 */
static bool registered(const StaticApplicationPlan *plan, const StaticApplicationRecord *record) {
    for (const StaticApplicationRecord *item = plan->first; item; item = item->next) {
        if (item == record) { return true; }
    }
    return false;
}

/** Independent facts may fill deferred fields but cannot change any known name,
 * category or whole-candidate residual contract.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) forbids incompatible interfaces.
 */
static bool compatible(StaticApplicationPlan *plan, const StaticSignature *previous,
    const StaticSignature *signature, SourceSpan site, const StaticSymbol *formal) {
    StaticSignatureComparison comparison;
    if (!static_constraint_signature(plan->solver, previous, signature, site, &comparison)) {
        (void)static_application_dependencies(plan); return false;
    }
    if (comparison == STATIC_SIGNATURE_DIFFERENT) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, site,
            "Expected metadata contradicts a known constructor signature", formal);
    }
    return true;
}

/** A resolved root description can already constrain this residual's nested
 * signature even when the original generic declaration was deferred.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves exact residuals.
 */
static bool residual_contract(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    const StaticSymbol *formal, const StaticSignature *signature, SourceSpan site) {
    const StaticTerm *value = static_constraint_root(plan->solver, record->value);
    if (value->category != STATIC_CATEGORY_TEMPLATE) { return true; }
    StaticCategory category;
    const StaticSignature *description;
    static_constraint_description(plan->solver, value, &category, &description);
    for (const StaticSignatureEntry *entry = description->first; entry; entry = entry->next) {
        if (static_same_name(entry->name, formal->name)) {
            return compatible(plan, entry->signature, signature, site, formal);
        }
    }
    return true;
}

/** Supplied metadata must be independent of the actual argument and immutable
 * until solver destruction; generic source records are never rewritten.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) fixes expected contracts.
 */
bool static_application_expect(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    const StaticSymbol *formal, const StaticSignature *signature, SourceSpan site) {
    if (!static_application_dependencies(plan)) { return false; }
    if (!registered(plan, record) || record->pending_origin || record->value == NULL ||
        !static_term_has_symbol(plan->terms, formal) || formal->role != STATIC_PARAMETER ||
        formal->category != STATIC_CATEGORY_TEMPLATE || signature == NULL || signature->count == 0) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, site, "Invalid independent expected signature fact", NULL);
    }
    StaticConstraintOutcome initial;
    if (!static_constraint_solve(plan->solver, &initial)) { (void)static_application_dependencies(plan); return false; }
    const StaticTerm *value = static_constraint_root(plan->solver, record->value);
    if (value->kind != STATIC_TERM_NOMINAL || formal->owner != value->as.nominal.origin) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, site, "Expected formal belongs to another candidate context", formal);
    }
    StaticSignatureComparison comparison;
    if (!static_constraint_signature(plan->solver, signature, signature, site, &comparison)) {
        (void)static_application_dependencies(plan); return false;
    }
    if (comparison != STATIC_SIGNATURE_EQUAL) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, site, "Expected signature fact is not fully resolved", formal);
    }
    const StaticSignature *previous = static_application_expected(plan, record, formal);
    if (!compatible(plan, previous, signature, site, formal) ||
        !residual_contract(plan, record, formal, signature, site)) { return false; }
    for (const StaticApplicationExpectation *fact = plan->first_expectation; fact; fact = fact->next) {
        if (fact->record == record && fact->formal == formal) { return true; }
    }
    if (plan->expectation_count == SIZE_MAX) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, site,
            "Expected signature fact count exceeds representable storage", formal);
    }
    StaticApplicationExpectation *fact = static_application_allocate(plan, sizeof(*fact));
    if (fact == NULL) { return false; }
    *fact = (StaticApplicationExpectation){.record = record, .formal = formal, .signature = signature,
        .site = site, .next = plan->first_expectation};
    plan->first_expectation = fact;
    plan->expectation_count++;
    for (const StaticApplicationInput *input = record->first; input; input = input->next) {
        if (input->generated && input->formal == formal &&
            !static_constraint_describe(plan->solver, input->value, STATIC_CATEGORY_TEMPLATE, signature, site)) {
            (void)static_application_dependencies(plan); return false;
        }
    }
    if (!static_application_residual_metadata(plan, record)) { return false; }
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(plan->solver, &solved)) { (void)static_application_dependencies(plan); return false; }
    StaticApplicationOutcome outcome;
    return static_application_validate(plan, record, &outcome);
}
