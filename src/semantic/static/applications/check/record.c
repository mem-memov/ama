/** @file
 * @brief Check inherited and new ordinary constructor bindings against original Type slots.
 * [TPL-02](../../../../../spec/language/templates.md#tpl-02) makes every ordinary formal Type.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves unused inherited captures.
 */
#include <assert.h>
#include "../internal.h"
#include "../../signature/usage/usage.h"

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

/** Pending metadata is not filled from its first actual. A known
 * category disagreement is an immediate error even when other slots are open.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) preserves actual metadata.
 */
static bool capture(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    const StaticTermCapture *binding, bool *pending) {
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan, binding->value, &category, &signature);
    if (category == STATIC_CATEGORY_DEFERRED) { *pending = true; return true; }
    assert(binding->formal->category == STATIC_CATEGORY_TYPE);
    if (category != STATIC_CATEGORY_TYPE) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, input_site(record, binding->formal),
            "Static argument has the wrong Type or Template category", binding->formal);
    }
    return true;
}

/** A supplied graph value must support every recorded use of its local name.
 * Other unresolved operands stay represented in the inventory for the producer
 * solver; this local snapshot never certifies whole-graph closure.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) forbids first-use contracts.
 */
static bool known_usage(StaticApplicationPlan *plan, const StaticApplicationRecord *record) {
    if (record->head->kind != STATIC_TERM_VARIABLE) { return true; }
    const StaticSymbol *symbol = record->head->as.variable.symbol;
    if (symbol == NULL || symbol->owner->definition->syntax.sections.kind != MODULE_STATIC) { return true; }
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan, record->head, &category, &signature);
    StaticUsageResult checked = static_usage_check(plan->terms->inventory, symbol, category, signature);
    return checked.status != STATIC_USAGE_CONFLICT || static_application_fail(plan, STATIC_APPLY_CONFLICT,
        checked.site, checked.message, symbol);
}

/** Construction may yield a useful symbolic candidate while a separate binding
 * check still waits for independent metadata. Closure/admission is not implied.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves symbolic dependencies.
 */
bool static_application_validate(StaticApplicationPlan *plan, const StaticApplicationRecord *record,
    StaticApplicationOutcome *output) {
    if (record->pending_origin || record->value == NULL) { *output = STATIC_APPLY_PENDING; return true; }
    if (!known_usage(plan, record)) { return false; }
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
