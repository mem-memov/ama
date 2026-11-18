/** @file
 * @brief Compose independently known residual contracts into solver-owned immutable descriptions.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) treats residuals as local binders.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) gives descriptors solver lifetime.
 */
#include "../internal.h"

/** Check independent nested metadata before allocating a complete root snapshot;
 * pending descriptions remain explicit and are never replaced by an actual arg.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves capture dependencies.
 */
bool static_application_residual_metadata(StaticApplicationPlan *plan, const StaticApplicationRecord *record) {
    const StaticTerm *value = static_constraint_root(plan->solver, record->value);
    if (value->kind != STATIC_TERM_NOMINAL || value->category != STATIC_CATEGORY_TEMPLATE) { return true; }
    StaticCategory category;
    const StaticSignature *previous;
    static_constraint_description(plan->solver, value, &category, &previous);
    StaticSignatureComparison comparison;
    if (!static_constraint_signature(plan->solver, previous, previous, record->context.site, &comparison)) {
        (void)static_application_dependencies(plan); return false;
    }
    if (comparison == STATIC_SIGNATURE_EQUAL) { return true; }
    for (const StaticTermResidual *slot = value->as.nominal.first_residual; slot; slot = slot->next) {
        if (slot->formal->category != STATIC_CATEGORY_TEMPLATE) { continue; }
        const StaticSignature *signature = static_application_expected(plan, record, slot->formal);
        if (!static_constraint_signature(plan->solver, signature, signature, record->context.site, &comparison)) {
            (void)static_application_dependencies(plan); return false;
        }
        if (comparison == STATIC_SIGNATURE_PENDING) { return true; }
    }
    StaticSignature *signature = static_constraint_allocate(plan->solver, sizeof(*signature));
    if (signature == NULL) { (void)static_application_dependencies(plan); return false; }
    for (const StaticTermResidual *slot = value->as.nominal.first_residual; slot; slot = slot->next) {
        StaticSignatureEntry *entry = static_constraint_allocate(plan->solver, sizeof(*entry));
        if (entry == NULL) { (void)static_application_dependencies(plan); return false; }
        entry->name = slot->formal->name;
        entry->category = slot->formal->category;
        const StaticSignature *expected = static_application_expected(plan, record, slot->formal);
        if (expected != NULL) {
            /* Copy the root descriptor, borrowing its immutable entries under
             * the independent fact's existing solver-lifetime contract. */
            entry->signature = static_constraint_allocate(plan->solver, sizeof(*entry->signature));
            if (entry->signature == NULL) { (void)static_application_dependencies(plan); return false; }
            *entry->signature = *expected;
        }
        if (signature->last != NULL) { signature->last->next = entry; }
        else { signature->first = entry; }
        signature->last = entry;
        signature->count++;
    }
    if (!static_constraint_describe(plan->solver, value, category, signature, record->context.site)) {
        (void)static_application_dependencies(plan); return false;
    }
    return true;
}
