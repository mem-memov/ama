/** @file
 * @brief Verify forward application laws independently of source expansion or runtime state.
 */
#include "../common.c"

/** Direct, staged and reordered maps have one normalized value; a skip stays
 * residual and later completion retains all fixed bindings, even unused ones.
 */
static void it_normalizes_direct_staged_reordered_and_skipped_original_maps(void) {
    setup_applications(NULL, NULL);
    const StaticSymbol *a = application_formal(target_module, 0), *b = application_formal(target_module, 1);
    StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
    StaticApplicationInput inputs[] = {{.formal = a, .value = integer_type, .site = a->site},
        {.formal = b, .value = integer_type, .site = b->site}};
    const StaticApplicationRecord *direct, *first, *staged, *reordered, *skip, *identity;
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
    inputs[0].value = NULL;
    assert(static_application_forward(&applications, target_constructor, inputs, 2, context, &skip));
    assert(skip->value->category == STATIC_CATEGORY_TEMPLATE && skip->value->as.nominal.first_residual->formal == a);
    assert(skip->first->value == NULL && skip->last->value == integer_type && skip->input_count == 2);
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

/** A Template value's local residuals remain valid arguments with an exact
 * named contract. A completely applied Type is never implicitly reopened.
 */
static void it_preserves_template_arguments_and_refuses_closed_type_reopening(void) {
    for (size_t closed = 0; closed < 2; closed++) {
        setup_applications("@<\n...a\n...b\n...family\n ...slot\n@!\nanchor&\n..integer\n",
            "@<\n...slot\n@!\n");
        const StaticSymbol *family = application_formal(caller_module, 2);
        const StaticTerm *actual = target_constructor;
        StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
        if (closed) {
            StaticApplicationInput slot = {.formal = application_formal(target_module, 0), .value = integer_type};
            const StaticApplicationRecord *complete;
            assert(static_application_forward(&applications, target_constructor, &slot, 1, context, &complete));
            actual = complete->value;
        }
        StaticApplicationInput input = {.formal = family, .value = actual, .site = family->site};
        const StaticApplicationRecord *record = NULL;
        bool success = static_application_forward(&applications, caller_constructor, &input, 1, context, &record);
        assert(success == !closed);
        if (!closed) {
            assert(record->value->category == STATIC_CATEGORY_TEMPLATE && record->value->as.nominal.capture_count == 1);
            assert(record->value->as.nominal.first_capture->value == target_constructor);
            StaticApplicationOutcome outcome;
            assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_RESOLVED);
        } else { assert(record == NULL && applications.status == STATIC_APPLY_CONFLICT); }
        destroy_applications();
    }
}

/** Exact residual names matter even for equally sized constructor interfaces. */
static void it_rejects_renamed_template_slots_without_guessing_an_adapter(void) {
    setup_applications("@<\n...a\n...b\n...family\n ...slot\n@!\nanchor&\n..integer\n",
        "@<\n...different slot\n@!\n");
    const StaticSymbol *family = application_formal(caller_module, 2);
    StaticApplicationInput input = {.formal = family, .value = target_constructor, .site = family->site};
    const StaticApplicationRecord *output = NULL;
    assert(!static_application_forward(&applications, caller_constructor, &input, 1,
        (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &output));
    assert(output == NULL && applications.status == STATIC_APPLY_CONFLICT);
    assert(strstr(applications.error.message, "signature") && applications.error.related_count == 1);
    destroy_applications();
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

/** Execute independent algebraic and kind/signature refusal partitions. */
static void run(void) {
    it_normalizes_direct_staged_reordered_and_skipped_original_maps();
    it_rejects_rebinding_duplicates_foreign_formals_and_wrong_categories();
    it_preserves_template_arguments_and_refuses_closed_type_reopening();
    it_rejects_renamed_template_slots_without_guessing_an_adapter();
    it_rechecks_inherited_captures_after_a_symbolic_identity_gets_an_origin();
}
/** Return success after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
