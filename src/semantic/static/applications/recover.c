/** @file
 * @brief Recover an unknown constructor from one known nominal result and exact declared residuals.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) retains all fixed captures.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) permits unique recovery without search.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial maps.
 */
#include <stdint.h>
#include "internal.h"
#include "../internal.h"

/** Full names are foreign interface identities, never suffixes or source-order
 * positions. [TPL-10](../../../../spec/language/templates.md#tpl-10) fixes residual sets.
 */
static bool named(const StaticSignature *signature, SourceSpan name) {
    for (const StaticSignatureEntry *entry = signature->first; entry; entry = entry->next) {
        if (static_same_name(entry->name, name)) { return true; }
    }
    return false;
}

/** Validate every expected name against the independently known actual origin,
 * and preserve every already-open actual slot. No partition is enumerated.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) requires S union P exactly.
 */
static bool names(StaticApplicationPlan *plan, const StaticTerm *actual,
    const StaticSignature *signature, SourceSpan site) {
    for (const StaticSignatureEntry *entry = signature->first; entry; entry = entry->next) {
        bool found = false;
        for (const StaticSymbol *formal = actual->as.nominal.origin->first; formal; formal = formal->next) {
            if (formal->role == STATIC_PARAMETER && static_same_name(entry->name, formal->name)) { found = true; break; }
        }
        if (!found) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, site,
                "Recovered constructor signature names no original formal of the actual origin", NULL);
        }
    }
    for (const StaticTermResidual *slot = actual->as.nominal.first_residual; slot; slot = slot->next) {
        if (!named(signature, slot->formal->name)) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, site,
                "Recovery cannot discard an existing actual residual formal", slot->formal);
        }
    }
    return true;
}

/** The declared residual set E determines the only possible candidate:
 * retain B outside E. Forward normalization then checks the actual applied set
 * P through its complete result equation, proving E = S union P. This is an
 * algebraic factorization of B minus P, not a choice among parameter partitions.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves uniqueness.
 */
static bool constructor(StaticApplicationPlan *plan, const StaticTerm *head,
    const StaticTerm *actual, const StaticSignature *signature, SourceSpan site) {
    if (!names(plan, actual, signature, site)) { return false; }
    size_t maximum = actual->as.nominal.capture_count;
    if (maximum > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, site,
            "Recovered capture map exceeds representable storage", NULL);
    }
    StaticTermCapture *captures = maximum ? static_application_allocate(plan, maximum * sizeof(*captures)) : NULL;
    if (maximum != 0 && captures == NULL) { return false; }
    size_t count = 0;
    for (const StaticTermCapture *capture = actual->as.nominal.first_capture; capture; capture = capture->next) {
        if (!named(signature, capture->formal->name)) {
            captures[count++] = (StaticTermCapture){.formal = capture->formal, .value = capture->value};
        }
    }
    const StaticTerm *candidate;
    if (!static_term_nominal(plan->terms, actual->as.nominal.origin, captures, count, site, &candidate)) {
        (void)static_application_dependencies(plan); return false;
    }
    StaticSignatureComparison comparison;
    if (!static_constraint_signature(plan->solver, signature, candidate->signature, site, &comparison)) {
        (void)static_application_dependencies(plan); return false;
    }
    if (comparison == STATIC_SIGNATURE_DIFFERENT) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, site,
            "Recovered constructor differs from its declared recursive signature", NULL);
    }
    if (!static_constraint_equate(plan->solver, head, candidate, site)) {
        (void)static_application_dependencies(plan); return false;
    }
    return true;
}

/** Record the result equality before the structural pass, so any newly known
 * head is checked against existing facts instead of overwritten. Finite unknown
 * applications remain explicit constraints for their later orchestration owner.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves all equations.
 */
bool static_application_recover(StaticApplicationPlan *plan, const StaticTermApplication *source,
    const StaticTerm *actual, const StaticApplicationRecord **output) {
    if (!static_application_dependencies(plan)) { return false; }
    const StaticTermApplication *registered = plan->terms->first_application;
    while (registered != NULL && registered != source) { registered = registered->next; }
    if (registered == NULL || output == NULL || !static_term_has_value(plan->terms, actual)) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, (SourceSpan){0},
            "Recovery requires an owned source application and actual value", NULL);
    }
    SourceSpan site = source->source->expression->span;
    const StaticTerm *result;
    if (!static_term_expression_result(plan->terms, source->environment, source->source->expression, &result) ||
        !static_constraint_equate(plan->solver, result, actual, site)) {
        (void)static_application_dependencies(plan); return false;
    }
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(plan->solver, &solved)) { (void)static_application_dependencies(plan); return false; }
    const StaticTerm *head = static_constraint_root(plan->solver, source->head);
    actual = static_constraint_root(plan->solver, actual);
    if (head->kind == STATIC_TERM_VARIABLE && actual->kind == STATIC_TERM_NOMINAL) {
        StaticCategory category;
        const StaticSignature *signature;
        static_application_description(plan, source->head, &category, &signature);
        if (category == STATIC_CATEGORY_TEMPLATE) {
            if (!constructor(plan, head, actual, signature, site)) { return false; }
        } else if (category == STATIC_CATEGORY_TYPE && source->input_count == 0) {
            if (!static_constraint_equate(plan->solver, head, actual, site)) {
                (void)static_application_dependencies(plan); return false;
            }
        }
    }
    return static_application_source(plan, source, output);
}
