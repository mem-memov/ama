/** @file
 * @brief Verify complete nested nominal facts, stable caches and no invented contexts.
 */

/** Exact equations for every original child must connect a known nested field
 * Type to its owner parameter without creating a caller root or expanding a body.
 */
static void it_infers_owner_types_from_nested_ordinary_operand_facts_without_actual_roots(void) {
    StaticOrdinaryType *type = prepare_operand_type(2);
    const StaticModule *inner = static_module_find(&inventory,
        project_find(&project, "/test/fixture/v1.0.0/src/inner"));
    const StaticModule *family = static_module_find(&inventory,
        project_find(&project, "/test/fixture/v1.0.0/src/family"));
    StaticTermCapture capture = {.formal = application_formal(inner, 0), .value = integer_type};
    const StaticTerm *inner_type, *expected;
    assert(static_term_nominal(&terms, inner, &capture, 1, (SourceSpan){0}, &inner_type));
    capture = (StaticTermCapture){.formal = application_formal(family, 0), .value = inner_type};
    assert(static_term_nominal(&terms, family, &capture, 1, (SourceSpan){0}, &expected));
    assert(static_constraint_equate(&solver, type->value, expected, type->expression->span));
    bool changed;
    assert(static_ordinary_equations(&publications, type, &changed));
    StaticConstraintOutcome solved;
    assert(static_constraint_solve(&solver, &solved));
    const StaticTerm *owner;
    StaticConstraintOutcome known;
    assert(static_constraint_value(&solver, application_symbol(".owner"), &owner, &known));
    assert(known == STATIC_CONSTRAINT_COMPLETE && owner == integer_type);
    assert(static_ordinary_equations(&publications, type, &changed) && !changed);
    assert(results.first_context == NULL && guards.first_path == NULL && result_registry.first_use == NULL);
    destroy_publications();
}

/** A leaf allocates no cursor; a deep original tree retains each child exactly
 * once and repeat collection neither allocates nor produces equation facts.
 */
static void it_collects_empty_and_deep_input_trees_once_without_equations_or_body_work(void) {
    const size_t depths[] = {0, 1024};
    for (size_t example = 0; example < sizeof(depths) / sizeof(*depths); example++) {
        size_t depth = depths[example];
        StaticOrdinaryType *type = prepare_operand_type(depth);
        size_t sources_before = results.source_count, equations_before = solver.equation_count;
        size_t before = allocation_count;
        assert(static_ordinary_collect_inputs(&publications, type) && type->inputs_collected);
        if (depth == 0) { assert(allocation_count == before && type->inputs == NULL); }
        assert(results.source_count == sources_before + depth);
        size_t count = 0, previous = 0;
        const StaticOrdinaryOperand *last = NULL;
        for (const StaticOrdinaryOperand *input = type->inputs; input; input = input->next) {
            assert(input->source->environment == caller_environment);
            assert(input->source->expression->head.start > previous);
            assert(input->prepared == NULL && input->application == NULL);
            previous = input->source->expression->head.start;
            last = input;
            count++;
        }
        assert(count == depth);
        if (last) { assert(layout_text_is(last->source->expression->head, ".owner")); }
        before = allocation_count;
        assert(static_ordinary_collect_inputs(&publications, type) && allocation_count == before);
        assert(solver.equation_count == equations_before && results.first_context == NULL);
        assert(guards.first_path == NULL && result_registry.first_use == NULL);
        destroy_publications();
    }
}

/** Equal constructor spellings in sibling branches retain distinct source
 * results; named slots, not written argument order, determine the actual map.
 */
static void it_keeps_branched_operand_sources_distinct_and_matches_reordered_named_maps(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/pair",
        "@<\nfirst\n.left\nsecond\n.right\n@!\n");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/inner", "@<\nitem\n.element\n@!\n");
    setup_publications("@<\nleft\n.left owner\nright\n.right owner\n@!\nanchor&\n//integer\n"
        "value\n./pair\n .right < ./inner\n  .element < .right owner\n"
        " .left < ./inner\n  .element < .left owner\n", "@!\n");
    StaticOrdinaryType *type;
    assert(static_ordinary_source(&publications, caller_environment, result_field("value"), &type));
    const StaticModule *inner = static_module_find(&inventory,
        project_find(&project, "/test/fixture/v1.0.0/src/inner"));
    const StaticModule *pair = static_module_find(&inventory,
        project_find(&project, "/test/fixture/v1.0.0/src/pair"));
    StaticTermCapture capture = {.formal = application_formal(inner, 0), .value = integer_type};
    const StaticTerm *left, *right, *expected;
    assert(static_term_nominal(&terms, inner, &capture, 1, (SourceSpan){0}, &left));
    capture.value = target_constructor;
    assert(static_term_nominal(&terms, inner, &capture, 1, (SourceSpan){0}, &right) && left != right);
    StaticTermCapture captures[] = {{.formal = application_formal(pair, 1), .value = right},
        {.formal = application_formal(pair, 0), .value = left}};
    assert(static_term_nominal(&terms, pair, captures, 2, (SourceSpan){0}, &expected));
    assert(static_constraint_equate(&solver, type->value, expected, type->expression->span));
    bool changed;
    assert(static_ordinary_equations(&publications, type, &changed));
    const char *heads[] = {"./inner", ".right owner", "./inner", ".left owner"};
    const StaticOrdinaryOperand *input = type->inputs;
    for (size_t index = 0; index < sizeof(heads) / sizeof(*heads); index++) {
        assert(input && layout_text_is(input->source->expression->head, heads[index]));
        assert(input->source->environment == caller_environment);
        input = input->next;
    }
    assert(input == NULL && type->inputs->source != type->inputs->next->next->source);
    const StaticTerm *actual;
    StaticConstraintOutcome known;
    assert(static_constraint_value(&solver, application_symbol(".left owner"), &actual, &known));
    assert(known == STATIC_CONSTRAINT_COMPLETE && actual == integer_type);
    assert(static_constraint_value(&solver, application_symbol(".right owner"), &actual, &known));
    assert(known == STATIC_CONSTRAINT_COMPLETE && actual == target_constructor);
    assert(results.first_context == NULL && guards.first_path == NULL && result_registry.first_use == NULL);
    destroy_publications();
}
