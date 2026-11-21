/** @file
 * @brief Exercise named declaration producers, nested selection and higher-order concrete Type composition.
 * The including leaf supplies prepare_sources and actual frontend ownership;
 * native fixture companions deliberately refuse any check-only C compilation.
 */
/** Every supplied declaration input participates in actual named normalization;
 * a nonexistent formal is rejected rather than disappearing during selection.
 */
static void it_rejects_unknown_named_declaration_producer_inputs(void) {
    prepare_sources("@!\nvalue\n./assembly\n .unknown < ./value\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.result\n./value\n");
    setup_frontend();
    assert(!check_project() && !frontend.valid);
    assert(diagnostic_valid(&frontend.error));
    assert(strcmp(frontend.error.message, "Unknown name in the actual interface sequence") == 0);
    assert(layout_text_is(frontend.error.primary.span, ".unknown"));
    destroy_frontend();
}

/** Finite nested selectors share the ordinary caller as lexical owner. Reusing
 * the same factory spelling inside its own written argument is not a factory
 * definition recurrence, and identical full maps share specialization identity.
 */
static void it_composes_nested_named_declaration_selections_in_the_real_frontend(void) {
    prepare_sources("@!\nvalue\n./assembly\n .input < ./assembly\n  .input < ./value\n  .result\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.result\n.input\n");
    setup_frontend();
    bool success = check_project();
    if (!success) { fprintf(stderr, "Nested declaration check failed: %s\n", frontend.error.message); }
    assert(success && frontend.valid);
    const ProjectModule *factory = project_find(&frontend.project, "/test/fixture/v1.0.0/src/assembly");
    size_t factory_count = 0;
    for (const StaticRegistryCandidate *candidate = frontend.specializations.registry.first_candidate;
        candidate; candidate = candidate->next) {
        if (candidate->key->definition->definition != factory) { continue; }
        factory_count++;
        assert(candidate->key->producer_applied && candidate->key->producer_inputs);
        assert(candidate->key->producer_inputs->value->category == STATIC_CATEGORY_TYPE);
        assert(candidate->key->producer_inputs->value->as.nominal.origin->definition ==
            project_find(&frontend.project, "/test/fixture/v1.0.0/src/value"));
    }
    assert(factory_count == 1);
    destroy_frontend();
}

/** A zero-input supplier returns a residual ordinary constructor. Another
 * supplier consumes that Template and applies a concrete Type, so only the
 * final fully applied ordinary result becomes the declared runtime Type.
 */
static void it_composes_selected_template_operands_into_one_concrete_declared_type(void) {
    prepare_sources("@!\nvalue&\n./assembly\n .family < ./partial\n  .result\n .type < ./value\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/family", "@<\nargument\n.slot\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/partial", "@...\n.result\n./family\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.result\n.family\n .slot < .type\n");
    setup_frontend();
    bool success = check_project();
    if (!success) { fprintf(stderr, "Selected Template composition failed: %s\n", frontend.error.message); }
    assert(success && frontend.valid);
    const ProjectModule *family = project_find(&frontend.project, "/test/fixture/v1.0.0/src/family");
    size_t family_count = 0;
    for (const StaticRegistryCandidate *candidate = frontend.specializations.registry.first_candidate;
        candidate; candidate = candidate->next) {
        if (candidate->key->definition->definition != family) { continue; }
        family_count++;
        assert(candidate->key->owner_type->category == STATIC_CATEGORY_TYPE);
        assert(candidate->key->owner_type->as.nominal.capture_count == 1);
        assert(candidate->key->owner_type->as.nominal.first_capture->value->as.nominal.origin->definition ==
            project_find(&frontend.project, "/test/fixture/v1.0.0/src/value"));
    }
    assert(family_count == 1);
    destroy_frontend();
}

/** Prepare a factory returning Node(T), whose reference field requests the
 * same factory at Box(T). With a concrete T this is different-key growth; with
 * an open T the repeated factory must suspend before another scope expands.
 */
static void prepare_recursive_declaration(bool concrete) {
    prepare_sources(concrete ?
        "@!\nnext&\n./assembly\n .input < ./value\n .result\n" :
        "@!\nnext&\n./assembly\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.result\n./node\n .slot < .input\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/node",
        "@<\nvalue\n.slot\n@!\nnext&\n./assembly\n .input < ./box\n  .slot < .slot\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@<\nvalue\n.slot\n@!\n");
    setup_frontend();
}

/** Source-only annotation expansion must retain the original factory path;
 * separate supplier roots would lose this known growing recurrence.
 */
static void it_rejects_growing_declaration_producer_keys_through_ordinary_metadata(void) {
    prepare_recursive_declaration(true);
    assert(!check_project() && !frontend.valid);
    StaticGuardPlan *guards = &frontend.specializations.guards;
    assert(guards->status == STATIC_GUARD_CONFLICT && guards->earlier_creation_key && guards->rejected_creation_key);
    assert(guards->rejected_origin->definition ==
        project_find(&frontend.project, "/test/fixture/v1.0.0/src/assembly"));
    assert(guards->creation_witness && guards->earlier_creation_key != guards->rejected_creation_key);
    destroy_frontend();
}

/** An unknown repeat cannot guess a concrete key or keep creating metadata
 * scopes. Only the original main declaration may have entered the factory;
 * Node's repeated request must have no new result context.
 */
static void it_suspends_open_declaration_producer_recursion_without_speculative_scopes(void) {
    prepare_recursive_declaration(false);
    assert(!check_project() && !frontend.valid);
    assert(frontend.specializations.guards.status == STATIC_GUARD_OK);
    const ProjectModule *factory = project_find(&frontend.project, "/test/fixture/v1.0.0/src/assembly");
    bool found = false;
    for (const StaticResultContext *context = frontend.specializations.results.first_context;
        context; context = context->next) {
        if (context->creation->owner->environment->definition->definition != factory) { continue; }
        found = true;
        assert(context->creation->parent);
        assert(context->creation->parent->owner->environment->definition->definition == frontend.project.entry);
        assert(context->registry_use->key_state == STATIC_REGISTRY_KEY_PENDING);
    }
    assert(found);
    destroy_frontend();
}

/** A closed factory delegates to an open producer, which creates an open Node.
 * Node asks for the same closed factory. Its delegated producer must retain the
 * unresolved annotation boundary despite the intervening closed factory scope.
 */
static void it_suspends_delegated_producers_below_open_ordinary_annotation_boundaries(void) {
    prepare_sources("@!\nnext&\n./assembly\n .input < ./value\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n./delegate\n .out > .result\n.keep\n.input\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/delegate",
        "@...\n.out\n./node\n .slot < .unbound\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/node",
        "@<\nvalue\n.slot\n@!\nnext&\n./assembly\n .input < ./value\n .result\n");
    setup_frontend();
    assert(!check_project() && !frontend.valid);
    assert(frontend.specializations.guards.status == STATIC_GUARD_OK);
    const ProjectModule *delegate = project_find(&frontend.project, "/test/fixture/v1.0.0/src/delegate");
    bool found = false;
    for (const StaticResultContext *context = frontend.specializations.results.first_context;
        context; context = context->next) {
        if (context->creation->owner->environment->definition->definition != delegate) { continue; }
        found = true;
        const StaticCreationPath *factory = context->creation->parent;
        assert(factory && factory->parent);
        assert(factory->parent->owner->environment->definition->definition == frontend.project.entry);
        assert(context->registry_use->key_state == STATIC_REGISTRY_KEY_PENDING);
    }
    assert(found);
    destroy_frontend();
}
