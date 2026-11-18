/** @file
 * @brief Verify pending expected interfaces use independent metadata rather than first-actual guessing.
 */
#include "../common.c"

/** Applying the declared one-slot inner constructor yields Type. T172 deliberately
 * leaves this application-dependent signature for the guarded later provider;
 * the known caller contract supplies a prepared independent slot-Type snapshot.
 */
static const char *caller_source = "@<\n...a\n...b\n...known\n ...slot\n"
    "@!\nanchor&\n..integer\nfield&\n.other\n";
static const char *target_source = "@<\n...inner\n ...value\n...element\n"
    "sample&\n...family\n ...slot < ...inner\n  ...value < ...element\n@!\n";

/** Captured and residual formals may await the same independent expected kind
 * computation, but neither takes its signature from a supplied actual's shape.
 */
static void it_refines_captured_and_residual_expectations_without_rewriting_source(void) {
    for (size_t captured = 0; captured < 2; captured++) {
        setup_applications(caller_source, target_source);
        const StaticSymbol *family = application_formal(target_module, 2);
        assert(family->category == STATIC_CATEGORY_TEMPLATE && family->signature->first->category == STATIC_CATEGORY_DEFERRED);
        const StaticTerm *known = application_symbol("...known");
        StaticApplicationInput input = {.formal = family, .value = known, .site = family->site};
        const StaticApplicationRecord *record;
        assert(static_application_forward(&applications, target_constructor, captured ? &input : NULL,
            captured ? 1 : 0, (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &record));
        StaticApplicationOutcome outcome;
        assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_PENDING);
        assert(static_application_expect(&applications, record, family, known->signature, family->site));
        assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_RESOLVED);
        size_t count = applications.expectation_count;
        assert(static_application_expect(&applications, record, family, known->signature, family->site));
        assert(applications.expectation_count == count && count == 1);
        size_t before = allocation_count;
        assert(static_application_expect(&applications, record, family, known->signature, family->site));
        assert(allocation_count == before);
        assert(family->signature->first->category == STATIC_CATEGORY_DEFERRED && inventory.valid);
        if (!captured) {
            StaticCategory category;
            const StaticSignature *signature;
            static_constraint_description(&solver, record->value, &category, &signature);
            assert(category == STATIC_CATEGORY_TEMPLATE && signature != record->value->signature);
            const StaticSignatureEntry *entry = signature->first;
            while (entry && !layout_text_is(entry->name, "...family")) { entry = entry->next; }
            assert(entry && entry->signature != known->signature);
            assert(entry->signature->first == known->signature->first);
            assert(entry->signature->count == known->signature->count);
        }
        destroy_applications();
    }
}

/** Generated Template holes get their own independently resolved declaration
 * contract; describing a hole does not manufacture a constructor origin.
 */
static void it_refines_generated_template_holes_without_choosing_a_constructor(void) {
    setup_applications(caller_source, target_source);
    const StaticSymbol *family = application_formal(target_module, 2);
    const StaticTerm *known = application_symbol("...known");
    const StaticOccurrence *field = NULL;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, "field")) { field = site; break; }
    }
    assert(field);
    StaticApplicationContext context;
    assert(static_application_context(&applications, caller_environment, field->expression, &context));
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &record));
    StaticApplicationOutcome outcome;
    assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_PENDING);
    assert(static_application_expect(&applications, record, family, known->signature, family->site));
    assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    const StaticApplicationInput *input = record->first;
    while (input && input->formal != family) { input = input->next; }
    assert(input && input->generated && input->value->category == STATIC_CATEGORY_TEMPLATE);
    StaticCategory category;
    const StaticSignature *signature;
    static_constraint_description(&solver, input->value, &category, &signature);
    assert(signature == known->signature && category == STATIC_CATEGORY_TEMPLATE);
    assert(static_constraint_root(&solver, input->value)->kind == STATIC_TERM_VARIABLE);
    StaticConstraintOutcome solved;
    assert(static_constraint_solve(&solver, &solved) && solved == STATIC_CONSTRAINT_PENDING);
    destroy_applications();
}

/** Repeated independent facts must preserve original full names. A copied
 * record/formal or a still-deferred description cannot supply trusted metadata.
 */
static void it_refuses_foreign_pending_and_conflicting_expected_signature_facts(void) {
    for (size_t partition = 0; partition < 4; partition++) {
        setup_applications(caller_source, target_source);
        const StaticSymbol *family = application_formal(target_module, 2);
        const StaticTerm *known = application_symbol("...known");
        const StaticApplicationRecord *record;
        assert(static_application_forward(&applications, target_constructor, NULL, 0,
            (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &record));
        StaticApplicationRecord copied_record = *record;
        StaticSymbol copied_formal = *family;
        const StaticSignature *signature = known->signature;
        if (partition == 2) { signature = family->signature; }
        if (partition == 3) { signature = application_formal(target_module, 0)->signature; }
        assert(!static_application_expect(&applications, partition == 0 ? &copied_record : record,
            partition == 1 ? &copied_formal : family, signature, family->site));
        assert(applications.expectation_count == 0);
        assert(applications.status == (partition == 3 ? STATIC_APPLY_CONFLICT : STATIC_APPLY_INVALID));
        assert(inventory.valid && family->signature->first->category == STATIC_CATEGORY_DEFERRED);
        destroy_applications();
    }
}

/** Every allocation in independent metadata composition is fallible. A fact may
 * already have been retained when a later solver snapshot fails; the failed
 * owner remains destructible and no successful check is falsely published.
 */
static void it_unwinds_every_independent_expected_metadata_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_applications(caller_source, target_source);
        const StaticSymbol *family = application_formal(target_module, 2);
        const StaticTerm *known = application_symbol("...known");
        const StaticApplicationRecord *record;
        assert(static_application_forward(&applications, target_constructor, NULL, 0,
            (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &record));
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool success = static_application_expect(&applications, record, family, known->signature, family->site);
        if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
        else {
            assert(!success && applications.status == STATIC_APPLY_OUT_OF_MEMORY);
            assert(applications.count == 1 && applications.first == record);
        }
        failure_at = 0;
        assert(inventory.valid && family->signature->first->category == STATIC_CATEGORY_DEFERRED);
        destroy_applications();
    }
}

/** Execute independent metadata-refinement and refusal partitions. */
static void run(void) {
    it_refines_captured_and_residual_expectations_without_rewriting_source();
    it_refines_generated_template_holes_without_choosing_a_constructor();
    it_refuses_foreign_pending_and_conflicting_expected_signature_facts();
    it_unwinds_every_independent_expected_metadata_allocation_refusal();
}
/** Return success after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
