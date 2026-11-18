/** @file
 * @brief Verify positional re-normalization preserves original RHS syntax and skips.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** The head fixes a; source positions must therefore select b and c. */
static const char *caller_source = "@<\n...constructor\n ...b\n ...c\n@>\n...family\n"
    "...constructor\n ... <\n ... < ..integer\n@!\nanchor&\n..integer\n";
static const char *target_source = "@<\n...a\n...b\n...c\n@!\n";

/** Select the original static result expression. */
static const AstTypeExpression *expression(void) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_STATIC_RESULT) {
            return site->expression;
        }
    }
    assert(false); return NULL;
}

/** Compose actual T173/T175 records while the request interface is unknown. */
static const StaticTermApplication *prepare(const StaticTerm *head) {
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, expression(), &requests));
    assert(requests->input_count == 2 && requests->pending);
    const BindingRequest *written = requests->first;
    assert(written && written->omitted && written->next && !written->next->omitted);
    StaticTermOperand operand = {.request = written->next, .value = integer_type};
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, caller_environment, head, &operand, 1, &source));
    return source;
}

/** Known and later discovered heads filter captures before positional binding. */
static void it_rebinds_positions_after_origin_discovery_without_reopening_captures(void) {
    for (size_t delayed = 0; delayed < 3; delayed++) {
        setup_applications(caller_source, target_source);
        StaticTermCapture fixed = {.formal = application_formal(target_module, 0), .value = integer_type};
        const StaticTerm *partial;
        assert(static_term_nominal(&terms, target_module, &fixed, 1, (SourceSpan){0}, &partial));
        const StaticTerm *head = delayed ? application_symbol("...constructor") : partial;
        const StaticTermApplication *source = prepare(head);
        const StaticApplicationRecord *record;
        if (delayed == 1) {
            assert(static_application_source(&applications, source, &record));
            assert(record->pending_origin && record->value == NULL && record->context.result != NULL);
            assert(record->context.source == source && record->context.requests == NULL);
            assert(static_constraint_equate(&solver, head, partial, expression()->span));
        }
        if (delayed == 2) {
            StaticTermCapture expected_captures[] = {fixed,
                {.formal = application_formal(target_module, 2), .value = integer_type}};
            const StaticTerm *expected;
            assert(static_term_nominal(&terms, target_module, expected_captures, 2, (SourceSpan){0}, &expected));
            assert(static_application_recover(&applications, source, expected, &record));
            assert(static_constraint_root(&solver, head) == partial);
        } else { assert(static_application_source(&applications, source, &record)); }
        assert(!record->pending_origin && record->input_count == 2 && record->context.requests != source->source);
        assert(record->first->formal == application_formal(target_module, 1) && record->first->value == NULL);
        assert(record->last->formal == application_formal(target_module, 2) && record->last->value == integer_type);
        assert(record->value->as.nominal.capture_count == 2 && record->value->as.nominal.residual_count == 1);
        assert(record->value->as.nominal.first_capture->formal == fixed.formal);
        assert(record->value->as.nominal.first_residual->formal == record->first->formal);
        assert(static_constraint_root(&solver, record->context.result) == record->value);
        StaticApplicationOutcome outcome;
        assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_RESOLVED);
        destroy_applications();
    }
}

/** Membership precedes following a copied record's internals. */
static void it_refuses_copied_source_records_without_publishing(void) {
    setup_applications(caller_source, target_source);
    const StaticTermApplication *source = prepare(target_constructor);
    StaticTermApplication copied = *source;
    const StaticApplicationRecord *output = NULL;
    assert(!static_application_source(&applications, &copied, &output));
    assert(output == NULL && applications.count == 0 && applications.status == STATIC_APPLY_INVALID);
    destroy_applications();
}

/** An output skip still creates the producer phase. Input positions count only
 * written inputs; an omitted input becomes a producer hole after normalization,
 * and static output selection uses its own original result sequence.
 */
static void it_preserves_separate_output_positions_and_uniform_producer_inference(void) {
    setup_applications("@<\n...constructor\n ...b\n ...c\n@>\n...family\n"
        "...constructor\n ... <\n ... < ..integer\n ... >\n@!\nanchor&\n..integer\n",
        "@<\n...a\n...b\n...c\n@>\n...result\n..integer\n@!\n");
    StaticTermCapture fixed = {.formal = application_formal(target_module, 0), .value = integer_type};
    const StaticTerm *partial;
    assert(static_term_nominal(&terms, target_module, &fixed, 1, expression()->span, &partial));
    const StaticTermApplication *source = prepare(partial);
    const StaticApplicationRecord *record;
    assert(static_application_source(&applications, source, &record));
    assert(record->context.policy == STATIC_OMIT_PRODUCER && record->input_count == 3);
    assert(record->context.requests->input_count == 2 && record->context.requests->output_count == 1);
    const BindingRequest *output = record->context.requests->last;
    assert(output->direction == BINDING_STATIC_OUTPUT && output->omitted);
    assert(output->slot->static_symbol == static_symbol_at(target_module, STATIC_RESULT, 0));
    assert(record->first->formal == application_formal(target_module, 1) && record->first->value == NULL);
    assert(record->first->next->formal == application_formal(target_module, 2));
    assert(record->first->next->value == integer_type);
    assert(record->last->generated && record->last->formal == record->first->formal);
    const StaticTerm *hole = record->last->value;
    assert(hole->as.variable.role == STATIC_VARIABLE_PRODUCER);
    assert(hole->as.variable.producer == expression() && hole != record->context.result);
    StaticConstraintOutcome solved;
    assert(static_constraint_solve(&solver, &solved) && solved == STATIC_CONSTRAINT_PENDING);
    assert(static_constraint_equate(&solver, hole, integer_type, expression()->span));
    assert(static_constraint_solve(&solver, &solved) && solved == STATIC_CONSTRAINT_COMPLETE);
    assert(caller_module->parameter_count == 1 && applications.count == 1);
    destroy_applications();
}

/** Execute source composition and foreign-owner refusal. */
static void run(void) {
    it_rebinds_positions_after_origin_discovery_without_reopening_captures();
    it_refuses_copied_source_records_without_publishing();
    it_preserves_separate_output_positions_and_uniform_producer_inference();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
