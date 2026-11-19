/** @file
 * @brief Verify namespace/member selection resource refusal, actual-scope checks and late conflicts.
 */
#include "../common.c"

/** Direct and independently exported namespaces sweep every observed selected read
 * allocation, including checked producer closure and declared member Type facts.
 */
static void it_unwinds_every_direct_and_exported_namespace_selection_allocation_refusal(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            if (mode) {
                directory(STATIC_SCOPE_ROOT_DIRECTORY);
                file("generated/static scope/container", "@=\nvalue\n...slot\n@<\ninput\n...slot\n@!\n");
            }
            setup_constants(mode ? "@!\n.other\n ...out\n value >> x\n" :
                "@!\nanchor&\n..integer\n.other\n value >> x\n", mode ?
                "@...\n...out\n.container\n ...slot < ..integer\n" : "@=\nvalue\n..integer\n@!\n");
            const StaticResultContext *context = publication_context(caller_environment);
            const StaticConstantRead *read;
            const StaticConstantSelection *selection = NULL;
            assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
            const StaticResultContext *actual;
            if (mode) {
                const StaticModule *container = static_module_find(&inventory, project_find(&project, ".container"));
                StaticTermCapture slot = {.formal = application_formal(container, 0), .value = integer_type};
                const StaticTerm *namespace_type;
                const StaticTermEnvironment *environment;
                bool prepared = static_term_nominal(&terms, container, &slot, 1, (SourceSpan){0}, &namespace_type);
                assert(prepared);
                prepared = static_term_environment(&terms, container, namespace_type, &environment);
                assert(prepared);
                actual = publication_context(environment);
            } else { actual = constant_fixed_target_context(); }
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = static_constant_select(&constants, read, actual, &selection);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && selection == NULL && constants.status == STATIC_CONSTANT_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_constants();
        }
    }
}

/** Foreign/copied handles, mismatched actual namespace scopes and count growth
 * refuse selection without publishing new member metadata to the caller.
 */
static void it_refuses_copied_reads_wrong_namespace_scopes_and_selection_counter_wrap(void) {
    for (size_t mode = 0; mode < 5; mode++) {
        setup_constants("@!\nanchor&\n..integer\n.other\n value >> x\n", "@=\nvalue\n..integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticResultContext *actual = constant_fixed_target_context();
        StaticResultContext copied_context = *context;
        const StaticConstantRead *read;
        const StaticConstantSelection *selection = NULL;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        StaticConstantRead copied = *read;
        if (mode == 4) { constants.selection_count = SIZE_MAX; }
        assert(!static_constant_select(&constants, mode == 0 ? &copied : read,
            mode == 1 ? &copied_context : mode == 2 ? context : actual, mode == 3 ? NULL : &selection));
        assert(selection == NULL && constants.status == (mode == 4 ? STATIC_CONSTANT_OUT_OF_MEMORY : STATIC_CONSTANT_INVALID));
        const char *message = constants.error.message;
        assert(!static_constant_select(&constants, read, NULL, &selection) && constants.error.message == message);
        destroy_constants();
    }
}

/** Late contradictory namespace evidence cannot return a prior descriptor as
 * though it were still consistent; old immutable metadata remains unchanged.
 */
static void it_rechecks_namespace_equations_before_reusing_existing_selections(void) {
    setup_constants("@!\nanchor&\n..integer\n.other\n value >> x\n", "@=\nvalue\n..integer\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticResultContext *actual = constant_fixed_target_context();
    const StaticConstantRead *read;
    const StaticConstantSelection *selection, *again = NULL;
    assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
    assert(static_constant_select(&constants, read, actual, &selection));
    assert(static_constraint_equate(&solver, read->source->source->source->value, caller_constructor, read->syntax->span));
    assert(!static_constant_select(&constants, read, actual, &again));
    assert(again == NULL && constants.status == STATIC_CONSTANT_CONFLICT && selection->namespace_type == target_constructor);
    assert(constants.selection_count == 1);
    destroy_constants();
}

/** A value label cannot silently select list storage. Independent shared Type
 * metadata and already failed borrowed owners remain separate source identities.
 */
static void it_checks_member_storage_labels_and_preserves_borrowed_original_errors(void) {
    setup_constants("@!\nanchor&\n..integer\n.other\n value >> x\n", "@=\nvalue[]\n..integer\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticResultContext *actual = constant_fixed_target_context();
    const StaticConstantRead *read;
    const StaticConstantSelection *selection = NULL;
    assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
    assert(!static_constant_select(&constants, read, actual, &selection));
    assert(selection == NULL && constants.status == STATIC_CONSTANT_CONFLICT && constants.declaration_count == 0);
    destroy_constants();
    setup_constant_fixture();
    context = publication_context(constant_applied_environment());
    const StaticConstantDeclaration *declaration;
    assert(static_constant_declaration(&constants, context, constant_named(caller_module, "scalar"), &declaration));
    assert(declaration->source->source->source->occurrence->symbol == result_named(caller_module, "...type"));
    assert(!static_constant_select(&constants, NULL, NULL, &selection));
    assert(constants.status == STATIC_CONSTANT_INVALID);
    destroy_constants();
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    static_constant_init(&constants, &publications, sources.allocator);
    context = publication_context(caller_environment);
    assert(!static_publication_resolve(&publications, context, result_pair("...first"), NULL));
    assert(!static_constant_select(&constants, NULL, NULL, &selection));
    assert(constants.error.message == publications.error.message && selection == NULL);
    destroy_constants();
}

/** Execute complete selection allocation, identity, storage and late-error checks. */
static void run(void) {
    it_unwinds_every_direct_and_exported_namespace_selection_allocation_refusal();
    it_refuses_copied_reads_wrong_namespace_scopes_and_selection_counter_wrap();
    it_rechecks_namespace_equations_before_reusing_existing_selections();
    it_checks_member_storage_labels_and_preserves_borrowed_original_errors();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
