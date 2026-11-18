/** @file
 * @brief Rebind original requests to independently known actual residual interfaces.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves capture maps.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) fixes actual positional order.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#include <stdint.h>
#include "internal.h"
#include "../bindings/normalize.h"

/** Unknown origins retain raw syntax rather than manufacturing original formal
 * IDs from an expected interface. The expression result can receive later facts.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) requires exact origins.
 */
static bool pending(StaticApplicationPlan *plan, const StaticTermApplication *source,
    StaticApplicationContext context, const StaticApplicationRecord **output) {
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan, source->head, &category, &signature);
    (void)signature;
    if (category == STATIC_CATEGORY_TYPE && source->input_count != 0) {
        return static_application_fail(plan, STATIC_APPLY_CONFLICT, context.site,
            "A Type cannot receive static input bindings", NULL);
    }
    if (source->input_count == 0 &&
        (category == STATIC_CATEGORY_TYPE || context.policy == STATIC_OMIT_PARTIAL)) {
        return static_application_forward(plan, source->head, NULL, 0, context, output);
    }
    if (plan->count == SIZE_MAX) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, context.site,
            "Static application count exceeds representable storage", NULL);
    }
    StaticApplicationRecord *record = static_application_allocate(plan, sizeof(*record));
    if (record == NULL) { return false; }
    *record = (StaticApplicationRecord){.head = source->head,
        .constructor = static_constraint_root(plan->solver, source->head),
        .context = context, .pending_origin = true};
    if (plan->last != NULL) { plan->last->next = record; }
    else { plan->first = record; }
    plan->last = record;
    plan->count++;
    *output = record;
    return true;
}

/** Match the same original input syntax after re-normalization; old request
 * ordinals and filtered slot pointers need not survive an origin discovery.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves request provenance.
 */
static const StaticTermInput *operand(const StaticTermApplication *source, const BindingRequest *request) {
    for (const StaticTermInput *input = source->first; input; input = input->next) {
        if (input->request->syntax.static_binding == request->syntax.static_binding) { return input; }
    }
    return NULL;
}

/** Validate owner membership before following borrowed records, snapshot the
 * actual residual interface, and forward only exact prepared RHS identities.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) forbids text substitution.
 */
bool static_application_source(StaticApplicationPlan *plan, const StaticTermApplication *source,
    const StaticApplicationRecord **output) {
    if (!static_application_dependencies(plan)) { return false; }
    const StaticTermApplication *registered = plan->terms->first_application;
    while (registered != NULL && registered != source) { registered = registered->next; }
    if (registered == NULL || output == NULL) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, (SourceSpan){0},
            "Source application must belong to the borrowed term store", NULL);
    }
    StaticApplicationContext context;
    if (!static_application_context(plan, source->environment, source->source->expression, &context)) { return false; }
    context.source = source;
    if (!static_term_expression_result(plan->terms, source->environment, context.expression, &context.result)) {
        (void)static_application_dependencies(plan); return false;
    }
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(plan->solver, &solved)) { (void)static_application_dependencies(plan); return false; }
    const StaticTerm *head = static_constraint_root(plan->solver, source->head);
    if (head->kind == STATIC_TERM_VARIABLE) { return pending(plan, source, context, output); }
    size_t fixed_count = head->as.nominal.capture_count;
    if (fixed_count > SIZE_MAX / sizeof(const StaticSymbol *) ||
        source->input_count > SIZE_MAX / sizeof(StaticApplicationInput)) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, context.site,
            "Source application snapshot exceeds representable storage", NULL);
    }
    const StaticSymbol **fixed = NULL;
    if (fixed_count != 0) {
        fixed = static_application_allocate(plan, fixed_count * sizeof(*fixed));
        if (fixed == NULL) { return false; }
    }
    size_t index = 0;
    for (const StaticTermCapture *capture = head->as.nominal.first_capture; capture; capture = capture->next) {
        fixed[index++] = capture->formal;
    }
    BindingInterface *view;
    BindingApplication *requests;
    if (!binding_source_interface(plan->bindings, head->as.nominal.origin, BINDING_MODULE_SURFACE,
        fixed, fixed_count, &view) ||
        !binding_normalize_static(plan->bindings, view, context.expression, &requests)) {
        (void)static_application_dependencies(plan); return false;
    }
    context.requests = requests;
    StaticApplicationInput *inputs = NULL;
    if (source->input_count != 0) {
        inputs = static_application_allocate(plan, source->input_count * sizeof(*inputs));
        if (inputs == NULL) { return false; }
    }
    index = 0;
    for (const BindingRequest *request = requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_INPUT) { continue; }
        const StaticTermInput *input = operand(source, request);
        if (input == NULL || request->slot == NULL || index == source->input_count) {
            return static_application_fail(plan, STATIC_APPLY_INVALID, request->span,
                "Normalized request has no exact original prepared operand", NULL);
        }
        inputs[index++] = (StaticApplicationInput){.formal = request->slot->static_symbol,
            .value = input->value, .site = request->span};
    }
    if (index != source->input_count) {
        return static_application_fail(plan, STATIC_APPLY_INVALID, context.site,
            "Normalized source input count changed", NULL);
    }
    return static_application_forward(plan, source->head, inputs, index, context, output);
}
