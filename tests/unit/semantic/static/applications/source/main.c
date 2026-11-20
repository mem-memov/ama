/** @file
 * @brief Compose named source requests with later actual origins and complete use requirements.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** Ordinary constructor formals are introduced through runtime name/type pairs. */
static const char *target_source = "@<\nfirst\n.a\nsecond\n.b\nthird\n.c\n@!\n";

/** Locate an original result expression by its public owner, not foreign labels. */
static const AstTypeExpression *expression(const char *result) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->root && site->statement->kind == AST_STATIC_RESULT
            && layout_text_is(site->statement->as.static_result.name, result)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Preserve one exact RHS term per named request while the actual head is open. */
static const StaticTermApplication *prepare(const char *result) {
    const AstTypeExpression *syntax = expression(result);
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, syntax, &requests));
    StaticTermOperand operands[2];
    size_t count = 0;
    for (const BindingRequest *input = requests->first; input; input = input->next) {
        assert(input->direction == BINDING_STATIC_INPUT && count < 2);
        operands[count++] = (StaticTermOperand){.request = input, .value = integer_type};
    }
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, caller_environment,
        application_symbol(".family"), operands, count, &source));
    return source;
}

/** Later actual origin discovery supplies full residual metadata. Named c,b
 * source order remains independent of the original a,b,c declaration order.
 */
static void it_renormalizes_named_requests_against_later_actual_residual_metadata(void) {
    setup_applications("@...\n.result\n.family\n .c < //integer\n .b < //integer\n"
        ".integer type\n//integer\n", target_source);
    const StaticTermApplication *source = prepare(".result");
    const StaticApplicationRecord *pending, *record;
    assert(static_application_source(&applications, source, &pending));
    assert(pending->pending_origin && pending->value == NULL && pending->context.result != NULL);
    StaticTermCapture fixed = {.formal = application_formal(target_module, 0), .value = integer_type};
    const StaticTerm *partial;
    assert(static_term_nominal(&terms, target_module, &fixed, 1, source->source->expression->span, &partial));
    assert(static_constraint_equate(&solver, source->head, partial, source->source->expression->span));
    assert(static_application_source(&applications, source, &record));
    assert(!record->pending_origin && record->value->category == STATIC_CATEGORY_TYPE);
    assert(record->context.requests != source->source && record->context.source == source);
    assert(record->first->formal == application_formal(target_module, 2));
    assert(record->last->formal == application_formal(target_module, 1));
    assert(record->first->value == integer_type && record->last->value == integer_type);
    assert(record->value->as.nominal.capture_count == 3);
    assert(record->value->as.nominal.first_capture->formal == fixed.formal);
    assert(record->value->as.nominal.first_capture->value == integer_type);
    assert(record->context.requests->first->source_ordinal == 0);
    assert(record->context.requests->first->slot->original_ordinal == 2);
    assert(source->source->first->slot == NULL && source->first->value == integer_type);
    StaticApplicationOutcome outcome;
    assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    destroy_applications();
}

/** Recovery preserves additional residuals needed by another use of the same
 * family; an actually missing name conflicts at that other original use.
 */
static void it_checks_all_minimum_uses_after_recovery_without_closing_the_signature(void) {
    for (size_t absent = 0; absent < 2; absent++) {
        const char *graph = absent
            ? "@...\n.first\n.family\n .b < //integer\n.second\n.family\n .missing < //integer\n"
            : "@...\n.first\n.family\n .b < //integer\n.second\n.family\n .c < //integer\n";
        setup_applications(graph, target_source);
        StaticTermCapture captures[] = {
            {.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}
        };
        const StaticTerm *actual;
        assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &actual));
        const StaticTermApplication *source = prepare(".first");
        const StaticApplicationRecord *record = NULL;
        bool success = static_application_recover(&applications, source, actual, &record);
        assert(success == !absent);
        if (absent) {
            assert(record == NULL && applications.status == STATIC_APPLY_CONFLICT);
            assert(layout_text_is(applications.error.primary.span, ".missing"));
        } else {
            assert(record->value == actual && record->value->category == STATIC_CATEGORY_TEMPLATE);
            const StaticTerm *head = static_constraint_root(&solver, source->head);
            assert(head->as.nominal.capture_count == 1 && head->as.nominal.residual_count == 2);
            assert(head->as.nominal.first_capture->formal == application_formal(target_module, 0));
        }
        destroy_applications();
    }
}

/** Static producer outputs need their own graph equations. A constant output
 * cannot establish a constructor origin or determine an omitted producer input.
 */
static void it_refuses_noninjective_producer_recovery_before_inventing_type_equations(void) {
    setup_applications("@...\n/test/fixture/v1.0.0/src/other\n .out > .alias\n.integer type\n//integer\n",
        "@...\n.out\n//integer\n.unused\n.free\n");
    const AstTypeExpression *syntax = caller_module->first_occurrence->expression;
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, syntax, &requests));
    const StaticTerm *head = application_symbol(".alias");
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, caller_environment, head, NULL, 0, &source));
    const StaticApplicationRecord *output = NULL;
    assert(!static_application_recover(&applications, source, integer_type, &output));
    assert(output == NULL && applications.status == STATIC_APPLY_INVALID);
    assert(solver.first_equation == NULL && static_constraint_root(&solver, head) == head);
    destroy_applications();
}

/** Execute actual request/origin and minimum-requirement compositions. */
static void run(void) {
    it_refuses_noninjective_producer_recovery_before_inventing_type_equations();
    it_renormalizes_named_requests_against_later_actual_residual_metadata();
    it_checks_all_minimum_uses_after_recovery_without_closing_the_signature();
}
/** Return success after complete counted teardown. */
int main(void) { run(); return 0; }
