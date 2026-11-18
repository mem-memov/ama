/** @file
 * @brief Check every inherited and new binding against its original declared category and signature.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) requires exact recursive contracts.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves unused inherited captures.
 */
#include "../internal.h"

/** Written/generated inputs retain their actual source sites; an inherited
 * capture falls back to this application site without a synthetic source.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves definition/application context.
 */
static SourceSpan input_site(const StaticApplicationRecord *record, const StaticSymbol *formal) {
    for (const StaticApplicationInput *input = record->first; input; input = input->next) {
        if (input->formal == formal && input->value != NULL) { return input->site; }
    }
    return record->context.site;
}

/** Pending metadata is not filled from its first actual. Known category or
 * signature disagreement is an immediate error even when other slots are open.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) fixes expected signatures independently.
 */
static bool capture(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    const StaticTermCapture *binding, bool *pending) {
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan, binding->value, &category, &signature);
    if (category == STATIC_CATEGORY_DEFERRED) { *pending = true; return true; }
    if (category != binding->formal->category) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, input_site(record, binding->formal),
            "Static argument has the wrong Type or Template category", binding->formal);
    }
    if (category == STATIC_CATEGORY_TEMPLATE) {
        StaticSignatureComparison comparison;
        if (!static_constraint_signature(plan->solver, static_application_expected(plan, record, binding->formal), signature,
            input_site(record, binding->formal), &comparison)) {
            (void)static_application_dependencies(plan);
            return false;
        }
        if (comparison == STATIC_SIGNATURE_DIFFERENT) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, input_site(record, binding->formal),
                "Static argument has a different residual constructor signature", binding->formal);
        }
        if (comparison == STATIC_SIGNATURE_PENDING) { *pending = true; }
    }
    return true;
}

/** Construction may yield a useful symbolic candidate while a separate binding
 * check still waits for independent metadata. Closure/admission is not implied.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves symbolic dependencies.
 */
bool static_application_validate(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    StaticApplicationOutcome *output) {
    if (record->pending_origin || record->value == NULL) { *output = STATIC_APPLY_PENDING; return true; }
    bool pending = false;
    const StaticTerm *value = static_constraint_root(plan->solver, record->value);
    if (value->kind == STATIC_TERM_NOMINAL) {
        for (const StaticTermCapture *binding = value->as.nominal.first_capture; binding; binding = binding->next) {
            if (!capture(plan, record, binding, &pending)) { return false; }
        }
    }
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan, record->value, &category, &signature);
    if (category == STATIC_CATEGORY_DEFERRED) { pending = true; }
    if (category == STATIC_CATEGORY_TEMPLATE) {
        StaticSignatureComparison comparison;
        if (!static_constraint_signature(plan->solver, signature, signature, record->context.site, &comparison)) {
            (void)static_application_dependencies(plan);
            return false;
        }
        if (comparison == STATIC_SIGNATURE_PENDING) { pending = true; }
    }
    *output = pending ? STATIC_APPLY_PENDING : STATIC_APPLY_RESOLVED;
    return true;
}

/** Only exact published snapshots are rechecked; copied structures and foreign
 * owners cannot acquire source/application identity.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves actual identities.
 */
bool static_application_check(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    StaticApplicationOutcome *output) {
    if (!static_application_dependencies(plan)) { return false; }
    if (output != NULL) {
        for (const StaticApplicationRecord *item = plan->first; item; item = item->next) {
            if (item != record) { continue; }
            StaticConstraintOutcome solved;
            if (!static_constraint_solve(plan->solver, &solved)) { (void)static_application_dependencies(plan); return false; }
            return static_application_validate(plan, record, output);
        }
    }
    return static_application_fail(plan, STATIC_APPLY_INVALID, (SourceSpan){0},
        "Static check requires a published application snapshot and output receiver", NULL);
}
