/** @file
 * @brief Verify independent concrete namespace selection and scoped own constant Type obligations.
 */
#include "../common.c"

/** Actual names/positions select only own declarations. Declared element Types
 * remain symbolic until their own type expressions/typed connections are solved.
 */
static void it_selects_named_and_positional_own_constants_before_completing_member_types(void) {
    const char *callers[] = {
        "@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n scalar >> x\n list[] >> xs[]\n",
        "@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n >> x\n >> xs[]\n"
    };
    for (size_t mode = 0; mode < 2; mode++) {
        setup_constants(callers[mode], "@=\nscalar\n//integer\nlist[]\n//integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticResultContext *actual = constant_fixed_target_context();
        const StaticConstantRead *read;
        const StaticConstantSelection *selection, *again;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        assert(static_constant_select(&constants, read, actual, &selection));
        assert(selection->namespace_type == target_constructor && selection->context == actual && read->source->source->producer == NULL);
        assert(selection->count == 2 && selection->first->declaration->syntax == constant_named(target_module, "scalar"));
        assert(selection->first->next->declaration->syntax == constant_named(target_module, "list"));
        assert(!selection->first->declaration->list && selection->first->next->declaration->list);
        assert(selection->first->declaration->source->source->source->head == integer_type);
        const StaticTerm *value = NULL;
        StaticConstraintOutcome outcome;
        assert(static_constraint_value(&solver, selection->first->declaration->type, &value, &outcome));
        assert(outcome == STATIC_CONSTRAINT_PENDING && value == NULL);
        assert(static_constant_select(&constants, read, actual, &again));
        assert(again == selection && constants.selection_count == 1 && constants.declaration_count == 2);
        destroy_constants();
    }
}

/** A closed static assembly supplies a concrete namespace before ordinary
 * member selection. Its producer is separate from the ordinary required scope.
 */
static void it_selects_a_concrete_export_before_namespace_member_selection(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/container", "@=\nempty[]\n.slot\n@<\ninput\n.slot\n@!\n");
    setup_constants("@!\n/test/fixture/v1.0.0/src/other\n .container type\n empty[] >> xs[]\n",
        "@...\n.container type\n/test/fixture/v1.0.0/src/container\n .slot < //integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantRead *read;
    const StaticConstantSelection *selection;
    bool success = static_constant_read(&constants, context, constant_namespace_read(0), &read);
    assert(success && read->source->source == NULL);
    const StaticTerm *namespace_type;
    success = static_ordinary_resolve(&publications, read->source, &namespace_type);
    assert(success && namespace_type->as.nominal.first_capture->value == integer_type);
    const StaticTermEnvironment *environment;
    success = static_term_environment(&terms, namespace_type->as.nominal.origin, namespace_type, &environment);
    assert(success);
    const StaticResultContext *actual = publication_context(environment);
    success = static_constant_select(&constants, read, actual, &selection);
    assert(success && selection->namespace_type == namespace_type && selection->first->declaration->list);
    const StaticTerm *element;
    StaticConstraintOutcome outcome;
    success = static_constraint_value(&solver, selection->first->declaration->source->source->source->head, &element, &outcome);
    assert(success && outcome == STATIC_CONSTRAINT_COMPLETE && element == integer_type);
    assert(selection->first->declaration->syntax->as.declaration.first == NULL);
    destroy_constants();
}

/** The static producer context is not the selected ordinary namespace's context.
 * Refusal must not pass a nonexistent producer Type to the constraint solver.
 */
static void it_refuses_a_static_producer_scope_as_an_ordinary_namespace_scope(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/container", "@=\nvalue\n//integer\n@!\n");
    setup_constants("@!\n/test/fixture/v1.0.0/src/other\n .out\n value >> x\n", "@...\n.out\n/test/fixture/v1.0.0/src/container\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantRead *read;
    bool success = static_constant_read(&constants, context, constant_namespace_read(0), &read);
    assert(success);
    const StaticTerm *type;
    success = static_ordinary_resolve(&publications, read->source, &type);
    assert(success && read->source->producer_context);
    const StaticConstantSelection *selection = NULL;
    success = static_constant_select(&constants, read, read->source->producer_context, &selection);
    assert(!success && selection == NULL && constants.status == STATIC_CONSTANT_INVALID);
    assert(publications.status == STATIC_PUBLICATION_OK && solver.status == STATIC_CONSTRAINT_OK);
    destroy_constants();
}

/** Instance results, body fields and an argument Type's constants never join
 * the selected namespace's own constant sequence.
 */
static void it_excludes_instance_fields_and_argument_type_constants_from_the_actual_surface(void) {
    const char *callers[] = {
        "@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n result >> x\n",
        "@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n hidden >> x\n",
        "@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n >> x\n >> y\n"
    };
    for (size_t mode = 0; mode < 3; mode++) {
        setup_constants(callers[mode], "@=\nshared\n//integer\n@>\nresult\n//integer\n@!\nhidden\n//integer\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticResultContext *actual = target_module->parameter_count == 0 ? constant_fixed_target_context() : NULL;
        const StaticConstantRead *read;
        const StaticConstantSelection *selection = NULL;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        assert(!static_constant_select(&constants, read, actual, &selection));
        assert(selection == NULL && constants.status == STATIC_CONSTANT_CONFLICT && constants.declaration_count == 0);
        destroy_constants();
    }
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/argument", "@=\nfrom argument\n//integer\n@!\n");
    setup_constants("@<\nnamespace\n.namespace\n@!\nanchor&\n//integer\nargument&\n/test/fixture/v1.0.0/src/argument\n.namespace\n from argument >> x\n",
        "@=\nonly own\n//integer\n@<\ninput\n.slot\n@!\n");
    const StaticModule *argument = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/argument"));
    const StaticTerm *argument_type, *namespace_type, *owner;
    bool success = static_term_nominal(&terms, argument, NULL, 0, (SourceSpan){0}, &argument_type);
    assert(success);
    StaticTermCapture slot = {.formal = application_formal(target_module, 0), .value = argument_type};
    success = static_term_nominal(&terms, target_module, &slot, 1, (SourceSpan){0}, &namespace_type);
    assert(success);
    StaticTermCapture supplied = {.formal = application_formal(caller_module, 0), .value = namespace_type};
    success = static_term_nominal(&terms, caller_module, &supplied, 1, (SourceSpan){0}, &owner);
    assert(success);
    const StaticTermEnvironment *environment;
    success = static_term_environment(&terms, caller_module, owner, &environment);
    assert(success);
    const StaticResultContext *context = publication_context(environment);
    success = static_term_environment(&terms, target_module, namespace_type, &environment);
    assert(success);
    const StaticResultContext *actual = publication_context(environment);
    const StaticConstantRead *read;
    const StaticConstantSelection *selection = NULL;
    success = static_constant_read(&constants, context, constant_namespace_read(0), &read);
    assert(success);
    success = static_constant_select(&constants, read, actual, &selection);
    assert(!success && selection == NULL && constants.status == STATIC_CONSTANT_CONFLICT);
    destroy_constants();
}

/** Known namespace/member shapes and typed receiver storage cannot complete
 * an open template or select an otherwise unknown family origin.
 */
static void it_refuses_member_or_receiver_driven_namespace_completion(void) {
    const char *callers[] = {
        "@!\nanchor&\n//integer\nxs[]\n//integer\n/test/fixture/v1.0.0/src/other\n empty[] >> xs[]\n",
        "@<\nnamespace\n.type\n@!\nanchor&\n//integer\nxs[]\n//integer\n.type\n empty[] >> xs[]\n"
    };
    for (size_t mode = 0; mode < 2; mode++) {
        setup_constants(callers[mode], "@=\nempty[]\n.slot\n@<\ninput\n.slot\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticConstantRead *read;
        const StaticConstantSelection *selection = NULL;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        assert(!static_constant_select(&constants, read, context, &selection));
        assert(selection == NULL && constants.status == (mode ? STATIC_CONSTANT_UNRESOLVED : STATIC_CONSTANT_CONFLICT));
        assert(constants.declaration_count == 0 && constants.selection_count == 0);
        destroy_constants();
    }
}

/** Duplicate own base names and shared reference storage are invalid even if
 * the selected request could otherwise resolve to an earlier valid declaration.
 */
static void it_rejects_duplicate_constant_names_and_unselected_reference_storage(void) {
    const char *targets[] = {
        "@=\nvalue\n//integer\nvalue[]\n//integer\n@!\n",
        "@=\nvalue\n//integer\nforbidden&\n//integer\n@!\n"
    };
    for (size_t mode = 0; mode < 2; mode++) {
        setup_constants("@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n value >> x\n", targets[mode]);
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticResultContext *actual = constant_fixed_target_context();
        const StaticConstantRead *read;
        const StaticConstantSelection *selection = NULL;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        assert(!static_constant_select(&constants, read, actual, &selection));
        assert(selection == NULL && constants.status == STATIC_CONSTANT_CONFLICT && constants.declaration_count == 0);
        assert(diagnostic_valid(&constants.error));
        destroy_constants();
    }
}

/** Plain Type forwarding does not invent a creation edge. Its actual required
 * namespace context must be supplied explicitly rather than found by a heuristic.
 */
static void it_requires_an_explicit_checked_registry_scope_for_plain_type_reuse(void) {
    for (size_t supplied = 0; supplied < 2; supplied++) {
        setup_constants("@<\nnamespace\n.type\n@!\nanchor&\n//integer\n.type\n value >> x\n", "@=\nvalue\n//integer\n@!\n");
        StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = target_constructor};
        const StaticTerm *owner;
        const StaticTermEnvironment *environment, *target_environment;
        assert(static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &owner));
        assert(static_term_environment(&terms, caller_module, owner, &environment));
        assert(static_term_environment(&terms, target_module, target_constructor, &target_environment));
        const StaticResultContext *context = publication_context(environment);
        const StaticResultContext *actual = publication_context(target_environment);
        const StaticConstantRead *read;
        const StaticConstantSelection *selection = NULL;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        bool success = static_constant_select(&constants, read, supplied ? actual : NULL, &selection);
        assert(success == (supplied != 0) && read->source->source->producer == NULL);
        if (success) { assert(selection->context == actual && selection->first->declaration->source->source->source->head == integer_type); }
        else { assert(selection == NULL && constants.status == STATIC_CONSTANT_INVALID); }
        destroy_constants();
    }
}

/** Copied selection handles and a wrong actual scope cannot manufacture member
 * identity; unrelated shared declarations retain their ordinary Type obligations.
 */
static void it_refuses_invalid_selection_handles_and_preserves_shared_type_scopes(void) {
    setup_constant_fixture();
    const StaticResultContext *context = publication_context(constant_applied_environment());
    const StaticConstantDeclaration *record;
    assert(static_constant_declaration(&constants, context, constant_named(caller_module, "scalar"), &record));
    assert(record->source->source->source->head == integer_type);
    assert(record->source->source->source->occurrence->symbol == result_named(caller_module, ".type"));
    const StaticConstantSelection *selection = NULL;
    assert(!static_constant_select(&constants, NULL, context, &selection));
    assert(selection == NULL && constants.status == STATIC_CONSTANT_INVALID);
    destroy_constants();
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    static_constant_init(&constants, &publications, sources.allocator);
    context = publication_context(caller_environment);
    assert(!static_publication_resolve(&publications, context, result_pair(".first"), NULL));
    assert(!static_constant_select(&constants, NULL, NULL, &selection));
    assert(constants.error.message == publications.error.message && selection == NULL);
    destroy_constants();
}

/** Execute namespace closure, own surfaces, symbolic member Types and registry scopes. */
static void run(void) {
    it_selects_named_and_positional_own_constants_before_completing_member_types();
    it_selects_a_concrete_export_before_namespace_member_selection();
    it_refuses_a_static_producer_scope_as_an_ordinary_namespace_scope();
    it_excludes_instance_fields_and_argument_type_constants_from_the_actual_surface();
    it_refuses_member_or_receiver_driven_namespace_completion();
    it_rejects_duplicate_constant_names_and_unselected_reference_storage();
    it_requires_an_explicit_checked_registry_scope_for_plain_type_reuse();
    it_refuses_invalid_selection_handles_and_preserves_shared_type_scopes();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
