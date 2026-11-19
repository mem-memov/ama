/** @file
 * @brief Refuse duplicate, foreign, captured and excess original static input slots.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids implicit reopening or adapters.
 */
#include "../internal.h"

/** Exact residual identity is independent of spelling and original allocation
 * order. [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves captures.
 */
static bool residual(const StaticTerm *constructor, const StaticSymbol *formal) {
    for (const StaticTermResidual *item = constructor->as.nominal.first_residual; item; item = item->next) {
        if (item->formal == formal) { return true; }
    }
    return false;
}

/** An unknown actual origin remains pending; input IDs themselves must still be
 * genuine accepted source formals and every supplied value must be an owned term.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves exact identities.
 */
bool static_application_inputs_valid(StaticApplicationPlan *plan, const StaticTerm *constructor,
    const StaticApplicationInput *inputs, size_t count, SourceSpan site) {
    if (count != 0 && inputs == NULL) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, site, "Missing prepared static input array", NULL);
    }
    if (constructor->kind == STATIC_TERM_NOMINAL && count > constructor->as.nominal.residual_count) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, site,
            "Static input bindings exceed the actual residual interface", NULL);
    }
    for (size_t index = 0; index < count; index++) {
        const StaticSymbol *formal = inputs[index].formal;
        if (!static_term_has_symbol(plan->terms, formal) || formal->role != STATIC_PARAMETER ||
            !static_term_has_value(plan->terms, inputs[index].value)) {
            return static_application_fail(plan, STATIC_APPLY_INVALID, inputs[index].site,
                "Prepared input requires original formal and owned value identities", NULL);
        }
        for (size_t previous = 0; previous < index; previous++) {
            if (inputs[previous].formal == formal) {
                return static_application_fail(plan, STATIC_APPLY_CONFLICT, inputs[index].site,
                    "Original static formal is supplied more than once", formal);
            }
        }
        if (constructor->kind != STATIC_TERM_NOMINAL) { continue; }
        if (formal->owner != constructor->as.nominal.origin) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, inputs[index].site,
                "Static input belongs to another original constructor", formal);
        }
        if (!residual(constructor, formal)) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, inputs[index].site,
                "A captured static formal cannot be rebound", formal);
        }
    }
    return true;
}
