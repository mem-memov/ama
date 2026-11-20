/** @file
 * @brief Exercise unique constructor recovery with retained captures and nested RHS equations.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** Independent lexical parameters and two known one-slot constructor signatures. */
static const char *caller_source = "@...\n.result\n.outer\n .b < .inner\n"
    "  .c < .element\n.integer type\n//integer\n";
static const char *target_source = "@<\nfirst argument\n.a\nsecond argument\n.b\n"
    "third argument\n.c\n@!\n";

/** Exact registered head spelling selects this fixture's outer/inner syntax. */
static const AstTypeExpression *expression(const char *name) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && layout_text_is(site->expression->head, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Import a single genuine explicit operand from its source binding request. */
static const StaticTermApplication *prepare(const char *name, const StaticTerm *operand) {
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, expression(name), &requests));
    assert(requests->input_count == 1 && requests->first->direction == BINDING_STATIC_INPUT);
    StaticTermOperand input = {.request = requests->first, .value = operand};
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, caller_environment,
        application_symbol(name), &input, 1, &source));
    return source;
}

/** Build independently specified closed values of one actual source origin. */
static const StaticTerm *actual(const StaticTerm *b) {
    StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
        {.formal = application_formal(target_module, 1), .value = b},
        {.formal = application_formal(target_module, 2), .value = integer_type}};
    const StaticTerm *value;
    assert(static_term_nominal(&terms, target_module, captures, 3, (SourceSpan){0}, &value));
    return value;
}

/** Equating the outer result propagates its b capture to the inner expression
 * result. Recovering the inner then fixes only its c slot, preserving a and b.
 */
static void it_composes_nested_recovery_without_losing_unused_fixed_bindings(void) {
    for (size_t early_inner = 0; early_inner < 2; early_inner++) {
        setup_applications(caller_source, target_source);
        const StaticTerm *inner_actual = actual(integer_type), *outer_actual = actual(inner_actual);
        const StaticTerm *element = application_symbol(".element"), *inner_result;
        assert(static_term_expression_result(&terms, caller_environment, expression(".inner"), &inner_result));
        const StaticTermApplication *inner = prepare(".inner", element);
        const StaticTermApplication *outer = prepare(".outer", inner_result);
        const StaticApplicationRecord *record;
        if (early_inner) {
            assert(static_application_recover(&applications, inner, inner_result, &record));
            assert(record->pending_origin && record->value == NULL);
        }
        assert(static_application_recover(&applications, outer, outer_actual, &record));
        assert(record->constructor->as.nominal.capture_count == 2);
        assert(record->constructor->as.nominal.first_residual->formal == application_formal(target_module, 1));
        assert(static_constraint_root(&solver, inner_result) == inner_actual);
        assert(static_application_recover(&applications, inner, inner_result, &record));
        assert(record->constructor->as.nominal.capture_count == 2);
        assert(record->constructor->as.nominal.first_residual->formal == application_formal(target_module, 2));
        assert(static_constraint_root(&solver, element) == integer_type);
        const StaticTerm *closed = NULL;
        StaticConstraintOutcome solved;
        assert(static_constraint_value(&solver, record->value, &closed, &solved));
        assert(solved == STATIC_CONSTRAINT_COMPLETE && closed == inner_actual);
        assert(static_application_recover(&applications, outer, outer_actual, &record));
        assert(static_constraint_value(&solver, record->value, &closed, &solved));
        assert(solved == STATIC_CONSTRAINT_COMPLETE && closed == outer_actual);
        assert(caller_module->parameter_count == 3 && target_module->parameter_count == 3);
        destroy_applications();
    }
}

/** A requested slot must be fixed in the actual. Already-open slots cannot be
 * assigned invented values merely to force the recovered constructor to fit.
 */
static void it_refuses_applied_slots_that_are_still_residual_in_the_actual(void) {
    setup_applications(caller_source, target_source);
    StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
        {.formal = application_formal(target_module, 2), .value = integer_type}};
    const StaticTerm *partial;
    assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &partial));
    const StaticTermApplication *source = prepare(".outer", integer_type);
    const StaticApplicationRecord *record = NULL;
    assert(!static_application_recover(&applications, source, partial, &record));
    assert(record == NULL && applications.status == STATIC_APPLY_CONFLICT);
    destroy_applications();
}

/** A previously solved constructor is immutable evidence. The later actual may
 * disagree with its retained capture even though origin and open names match.
 */
static void it_checks_existing_constructor_bindings_instead_of_overwriting_them(void) {
    setup_applications(caller_source, target_source);
    const StaticTerm *different = actual(integer_type);
    StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = different},
        {.formal = application_formal(target_module, 2), .value = integer_type}};
    const StaticTerm *bound;
    assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &bound));
    const StaticTerm *head = application_symbol(".outer");
    assert(static_constraint_equate(&solver, head, bound, expression(".outer")->span));
    const StaticTermApplication *source = prepare(".outer", integer_type);
    const StaticApplicationRecord *record = NULL;
    assert(!static_application_recover(&applications, source, different, &record));
    assert(record == NULL && applications.status == STATIC_APPLY_CONFLICT);
    destroy_applications();
}

/** Execute finite nested composition and conflicting decomposition partitions. */
static void run(void) {
    it_composes_nested_recovery_without_losing_unused_fixed_bindings();
    it_refuses_applied_slots_that_are_still_residual_in_the_actual();
    it_checks_existing_constructor_bindings_instead_of_overwriting_them();
}
/** Return success after complete counted teardown. */
int main(void) { run(); return 0; }
