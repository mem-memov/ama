/** @file
 * @brief Discover shared nominal key values in canonical postorder without recursive expansion.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) keeps ordinary constructor formals Type-valued.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains captures/residual metadata.
 * [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05) requires exact finite traversal.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed scratch.
 */
#include <stdint.h>
#include "internal.h"

/** Applied environments already own maps in original declaration order.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) ignores call ordering.
 */
const StaticTermCapture *static_key_inputs(const StaticTermEnvironment *environment) {
    return environment->producer_applied ? environment->producer_inputs : environment->owner_type->as.nominal.first_capture;
}

/** Exact canonical child pointers identify sharing inside one immutable store;
 * their addresses and construction ordinals are not emitted.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves equal structure.
 */
const StaticKeyNode *static_key_find(const StaticKeyPlan *plan, const StaticTerm *value) {
    for (const StaticKeyNode *node = plan->seen; node; node = node->next_seen) {
        if (node->value == value) { return node; }
    }
    return NULL;
}

/** Ordinary constructor parameters/residuals are Type-valued. A Template can
 * occur as a producer input but not as a capture in an ordinary constructor.
 * No body operation is checked or executed here.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves binding categories.
 */
static bool valid(const StaticTerm *value) {
    if (value->kind != STATIC_TERM_NOMINAL || !value->ground ||
        (value->category != STATIC_CATEGORY_TYPE && value->category != STATIC_CATEGORY_TEMPLATE)) { return false; }
    for (const StaticTermCapture *capture = value->as.nominal.first_capture; capture; capture = capture->next) {
        if (capture->value->category != STATIC_CATEGORY_TYPE) { return false; }
    }
    for (const StaticTermResidual *residual = value->as.nominal.first_residual; residual; residual = residual->next) {
        if (residual->formal->category != STATIC_CATEGORY_TYPE || residual->formal->signature != NULL) { return false; }
    }
    return true;
}

/** One scratch node also stores its DFS continuation. Finishing appends it once
 * after all children; shared finished nodes need no second expansion.
 * [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05) bounds visits by the finite DAG.
 */
static StaticKeyStatus discover(StaticKeyPlan *plan, const StaticTerm *value) {
    if (static_key_find(plan, value)) { return STATIC_KEY_OK; }
    StaticKeyNode *current = NULL;
    for (;;) {
        if (!valid(value)) { return STATIC_KEY_INVALID; }
        StaticKeyNode *node = source_arena_allocate(&plan->scratch, sizeof(*node));
        if (node == NULL) { return STATIC_KEY_OUT_OF_MEMORY; }
        *node = (StaticKeyNode){.value = value, .parent = current,
            .remaining = value->as.nominal.first_capture, .next_seen = plan->seen};
        plan->seen = node;
        current = node;
        while (current) {
            if (current->remaining) {
                const StaticTermCapture *capture = current->remaining;
                current->remaining = capture->next;
                const StaticKeyNode *child = static_key_find(plan, capture->value);
                if (child && !child->complete) { return STATIC_KEY_INVALID; }
                if (child) { continue; }
                value = capture->value;
                break;
            }
            if (plan->count == SIZE_MAX) { return STATIC_KEY_LIMIT; }
            current->ordinal = plan->count++;
            current->complete = true;
            if (plan->last) { plan->last->next_order = current; }
            else { plan->first = current; }
            plan->last = current;
            current = current->parent;
        }
        if (current == NULL) { return STATIC_KEY_OK; }
    }
}

/** Environment membership precedes dereference. Generic scopes and unresolved
 * values cannot be serialized as concrete specialization identities.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) requires complete maps.
 */
StaticKeyStatus static_key_collect(StaticKeyPlan *plan) {
    const StaticTermEnvironment *environment = plan->environment;
    if (plan->terms->status != STATIC_TERM_OK || !static_term_has_environment(plan->terms, environment)) {
        return STATIC_KEY_INVALID;
    }
    bool producer = environment->definition->definition->syntax.sections.kind == MODULE_STATIC;
    if (producer != environment->producer_applied ||
        (producer ? environment->owner_type != NULL : environment->owner_type == NULL)) { return STATIC_KEY_INVALID; }
    for (const StaticTermCapture *input = static_key_inputs(environment); input; input = input->next) {
        if (!producer && input->value->category != STATIC_CATEGORY_TYPE) { return STATIC_KEY_INVALID; }
        StaticKeyStatus status = discover(plan, input->value);
        if (status != STATIC_KEY_OK) { return status; }
    }
    return STATIC_KEY_OK;
}
