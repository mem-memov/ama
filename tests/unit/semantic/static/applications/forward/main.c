/** @file
 * @brief Verify forward application laws independently of source expansion or runtime state.
 */
#include "../common.c"

/** Direct, staged and reordered maps have one normalized value; an omitted formal stays
 * residual and later completion retains all fixed bindings, even unused ones.
 */
static void it_normalizes_direct_staged_reordered_and_omitted_original_maps(void) {
    setup_applications(NULL, NULL);
    const StaticSymbol *a = application_formal(target_module, 0), *b = application_formal(target_module, 1);
    StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
    StaticApplicationInput inputs[] = {{.formal = a, .value = integer_type, .site = a->site},
        {.formal = b, .value = integer_type, .site = b->site}};
    const StaticApplicationRecord *direct, *first, *staged, *reordered, *omitted, *identity;
    assert(static_application_forward(&applications, target_constructor, inputs, 2, context, &direct));
    assert(static_application_forward(&applications, target_constructor, inputs, 1, context, &first));
    assert(first->value->category == STATIC_CATEGORY_TEMPLATE && first->value->as.nominal.residual_count == 1);
    assert(static_application_forward(&applications, first->value, &inputs[1], 1, context, &staged));
    StaticApplicationInput reverse[] = {inputs[1], inputs[0]};
    assert(static_application_forward(&applications, target_constructor, reverse, 2, context, &reordered));
    assert(direct->value == staged->value && direct->value == reordered->value && direct->applied);
    assert(direct->value->as.nominal.first_capture->formal == a);
    assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &identity));
    assert(identity->value == target_constructor && !identity->applied);
    assert(static_application_forward(&applications, target_constructor, &inputs[1], 1, context, &omitted));
    assert(omitted->value->category == STATIC_CATEGORY_TEMPLATE && omitted->value->as.nominal.first_residual->formal == a);
    assert(omitted->first->formal == b && omitted->first->value == integer_type && omitted->input_count == 1);
    StaticApplicationOutcome outcome;
    assert(static_application_check(&applications, direct, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    assert(target_module->parameter_count == 2 && inventory.valid);
    destroy_applications();
}

/** Inherited capture identity cannot be reopened by an equal repeated value;
 * Type arguments and constructor signatures are separate static categories.
 */
static void it_rejects_rebinding_duplicates_foreign_formals_and_wrong_categories(void) {
    for (size_t partition = 0; partition < 4; partition++) {
        setup_applications(NULL, NULL);
        const StaticSymbol *a = application_formal(target_module, 0), *b = application_formal(target_module, 1);
        StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
        StaticApplicationInput inputs[] = {{.formal = a, .value = integer_type, .site = a->site},
            {.formal = b, .value = integer_type, .site = b->site}};
        const StaticApplicationRecord *prior, *output;
        assert(static_application_forward(&applications, target_constructor, inputs, 1, context, &prior));
        output = prior;
        const StaticTerm *head = target_constructor;
        size_t count = 1;
        if (partition == 0) { head = prior->value; }
        if (partition == 1) { inputs[1] = inputs[0]; count = 2; }
        if (partition == 2) { inputs[0].formal = application_formal(caller_module, 0); }
        if (partition == 3) { inputs[0].value = target_constructor; }
        assert(!static_application_forward(&applications, head, inputs, count, context, &output));
        assert(output == prior && applications.count == 1 && applications.status == STATIC_APPLY_CONFLICT);
        assert(prior->next == NULL && diagnostic_valid(&applications.error) && inventory.valid);
        destroy_applications();
    }
}



/** A bare Type reference can normalize before its origin is known, but later
 * checking must examine its resolved inherited captures rather than skipping
 * them merely because the original value record was a variable.
 */
static void it_rechecks_inherited_captures_after_a_symbolic_identity_gets_an_origin(void) {
    setup_applications(NULL, NULL);
    const StaticTerm *type = application_symbol("...a");
    const StaticApplicationRecord *identity;
    assert(static_application_forward(&applications, type, NULL, 0,
        (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &identity));
    assert(identity->value == type && !identity->applied);
    StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = target_constructor},
        {.formal = application_formal(target_module, 1), .value = integer_type}};
    const StaticTerm *invalid;
    assert(static_term_nominal(&terms, target_module, captures, 2, type->site, &invalid));
    assert(static_constraint_equate(&solver, type, invalid, type->site));
    StaticApplicationOutcome outcome = STATIC_APPLY_PENDING;
    assert(!static_application_check(&applications, identity, &outcome));
    assert(outcome == STATIC_APPLY_PENDING && applications.status == STATIC_APPLY_CONFLICT);
    assert(applications.error.related_count == 1 && inventory.valid);
    destroy_applications();
}

/** Omitted formals are absent, while a supplied null value is an invalid map.
 * A fully applied Type can be forwarded but can never be reopened as Template.
 */
static void it_refuses_missing_values_and_closed_type_reopening(void) {
    for (size_t partition = 0; partition < 2; partition++) {
        setup_applications(NULL, NULL);
        StaticApplicationInput inputs[] = {
            {.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}
        };
        StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
        const StaticApplicationRecord *complete, *output;
        assert(static_application_forward(&applications, target_constructor, inputs, 2, context, &complete));
        output = complete;
        const StaticTerm *head = partition == 0 ? target_constructor : complete->value;
        if (partition == 0) { inputs[0].value = NULL; }
        assert(!static_application_forward(&applications, head, inputs, 1, context, &output));
        assert(output == complete && applications.count == 1 && complete->next == NULL);
        assert(applications.status == (partition == 0 ? STATIC_APPLY_INVALID : STATIC_APPLY_CONFLICT));
        destroy_applications();
    }
}

/** Execute independent algebraic and kind/signature refusal partitions. */
static void run(void) {
    it_normalizes_direct_staged_reordered_and_omitted_original_maps();
    it_rejects_rebinding_duplicates_foreign_formals_and_wrong_categories();
    it_refuses_missing_values_and_closed_type_reopening();
    it_rechecks_inherited_captures_after_a_symbolic_identity_gets_an_origin();
}
/** Return success after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
