/** @file
 * @brief Verify key equality under actual partial application and solver aliases, plus full-key distinctions.
 */
#include "../common.c"

/** Select one original static result expression, preserving lexical ownership. */
static const AstTypeExpression *result_expression(const char *name) {
    const StaticSymbol *symbol = static_symbol_find(caller_module, static_name_site(project.entry, name));
    assert(symbol && symbol->role == STATIC_RESULT);
    return symbol->declaration->as.static_result.expression;
}

/** Resolve exact named input maps for their actual source expression. */
static const StaticApplicationRecord *apply_result(const char *name, const StaticTerm *head,
    const StaticApplicationInput *inputs, size_t count) {
    StaticApplicationContext context;
    assert(static_application_context(&applications, caller_environment, result_expression(name), &context));
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, head, inputs, count, context, &record));
    StaticApplicationOutcome outcome;
    assert(static_application_check(&applications, record, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    return record;
}

/** Staged and direct named applications in the actual graph yield one Template;
 * a solved producer-input alias must encode the same full key as that Template.
 */
static void it_preserves_named_partial_staging_and_solved_alias_identity(void) {
    setup_guards("@...\n.echo\n.argument\n.partial\n/test/fixture/v1.0.0/src/other\n .a < //integer\n"
        ".staged\n.partial\n .b < //integer\n.direct\n/test/fixture/v1.0.0/src/other\n .b < //integer\n .a < //integer\n",
        "@<\nfirst\n.a\nsecond\n.b\nthird\n.c\n@!\n");
    StaticApplicationInput first = {.formal = application_formal(target_module, 0), .value = integer_type};
    const StaticApplicationRecord *partial = apply_result(".partial", target_constructor, &first, 1);
    const StaticSymbol *partial_symbol = static_symbol_find(caller_module, static_name_site(project.entry, ".partial"));
    const StaticTerm *alias;
    assert(static_term_symbol(&terms, caller_environment, partial_symbol, &alias));
    assert(static_constraint_equate(&solver, alias, partial->value, partial_symbol->site));
    StaticApplicationInput second = {.formal = application_formal(target_module, 1), .value = integer_type};
    const StaticApplicationRecord *staged = apply_result(".staged", alias, &second, 1);
    StaticApplicationInput reordered[] = {second, first};
    const StaticApplicationRecord *direct = apply_result(".direct", target_constructor, reordered, 2);
    assert(staged->value == direct->value && direct->value->category == STATIC_CATEGORY_TEMPLATE);
    assert(direct->value->as.nominal.capture_count == 2 && direct->value->as.nominal.residual_count == 1);
    const StaticSymbol *argument = application_formal(caller_module, 0);
    const StaticTerm *input_alias;
    assert(static_term_symbol(&terms, caller_environment, argument, &input_alias));
    assert(static_constraint_equate(&solver, input_alias, direct->value, argument->site));
    StaticTermCapture inputs[] = {{.formal = argument, .value = staged->value}, {.formal = argument, .value = input_alias}};
    const StaticTermEnvironment *environments[2];
    for (size_t index = 0; index < 2; index++) {
        assert(static_term_producer_environment(&terms, caller_module, &inputs[index], 1, &environments[index]));
    }
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    StaticKeyEncoding left = key_encode(&destination, environments[0]);
    StaticKeyEncoding right = key_encode(&destination, environments[1]);
    assert(left.length == right.length && strcmp(left.descriptor, right.descriptor) == 0);
    IdentityRegistry names;
    identity_registry_init(&names, sources.allocator);
    const IdentityName *first_name, *second_name;
    assert(identity_register(&names, left.qualified, &first_name) == IDENTITY_OK);
    assert(identity_register(&names, right.qualified, &second_name) == IDENTITY_OK && first_name == second_name);
    identity_registry_destroy(&names);
    source_arena_destroy(&destination);
    destroy_guards();
}

/** Same-arity residual sets and fixed maps retain their original formal names;
 * root inputs remain part of identity even if the tested output only forwards.
 */
static void it_distinguishes_equal_arity_residuals_and_all_fixed_captures(void) {
    setup_guards("@...\n.echo\n.argument\n.anchor\n//integer\n",
        "@<\nfirst\n.a\nsecond\n.b\nthird\n.c\n@!\n");
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    StaticKeyEncoding keys[3];
    for (size_t ordinal = 0; ordinal < 3; ordinal++) {
        StaticTermCapture fixed = {.formal = application_formal(target_module, ordinal), .value = integer_type};
        const StaticTerm *partial;
        assert(static_term_nominal(&terms, target_module, &fixed, 1, (SourceSpan){0}, &partial));
        assert(partial->as.nominal.residual_count == 2);
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = partial};
        const StaticTermEnvironment *environment;
        assert(static_term_producer_environment(&terms, caller_module, &input, 1, &environment));
        keys[ordinal] = key_encode(&destination, environment);
        for (size_t previous = 0; previous < ordinal; previous++) {
            assert(strcmp(keys[ordinal].descriptor, keys[previous].descriptor) != 0);
        }
    }
    source_arena_destroy(&destination);
    destroy_guards();
}

/** Changing only a residual formal name changes full Template metadata even
 * when origin, fixed captures and remaining arity are otherwise identical.
 */
static void it_keeps_full_residual_names_beyond_origin_capture_and_arity(void) {
    char previous[1024] = {0};
    for (size_t renamed = 0; renamed < 2; renamed++) {
        setup_guards("@...\n.echo\n.argument\n.anchor\n//integer\n", renamed ?
            "@<\nfirst\n.a\nsecond\n.renamed\n@!\n" :
            "@<\nfirst\n.a\nsecond\n.b\n@!\n");
        StaticTermCapture fixed = {.formal = application_formal(target_module, 0), .value = integer_type};
        const StaticTerm *partial;
        assert(static_term_nominal(&terms, target_module, &fixed, 1, (SourceSpan){0}, &partial));
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = partial};
        const StaticTermEnvironment *environment;
        assert(static_term_producer_environment(&terms, caller_module, &input, 1, &environment));
        SourceArena destination;
        source_arena_init(&destination, sources.allocator);
        StaticKeyEncoding key = key_encode(&destination, environment);
        assert(key.length < sizeof(previous));
        if (renamed) { assert(strcmp(previous, key.descriptor) != 0); }
        else { memcpy(previous, key.descriptor, key.length + 1); }
        source_arena_destroy(&destination);
        destroy_guards();
    }
}

/** A project type and a standard type with the same basename retain different
 * explicit root tags, even when both have no bindings or runtime layout fields.
 */
static void it_preserves_root_kind_for_identical_module_basenames(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/integer", "@!\n");
    setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\nproject integer&\n/test/fixture/v1.0.0/src/integer\n", "@!\n");
    const StaticModule *module = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/integer"));
    const StaticTerm *project_type;
    assert(module && static_term_nominal(&terms, module, NULL, 0, (SourceSpan){0}, &project_type));
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    StaticKeyEncoding standard = key_encode(&destination, key_ordinary(caller_module, &input, 1));
    input.value = project_type;
    StaticKeyEncoding project_key = key_encode(&destination, key_ordinary(caller_module, &input, 1));
    assert(strcmp(standard.descriptor, project_key.descriptor) != 0);
    assert(strstr(standard.descriptor, "T29://integer") && strstr(project_key.descriptor, "T132:/test/fixture/v1.0.0/src/integer"));
    source_arena_destroy(&destination);
    destroy_guards();
}

/** Unrelated interned values change construction ordinals but cannot change
 * the source-ordered descriptor of the same complete reachable key.
 */
static void it_ignores_unrelated_allocation_and_term_construction_order(void) {
    char previous[2048] = {0};
    size_t previous_ordinal = 0;
    for (size_t noise = 0; noise < 2; noise++) {
        setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@<\nvalue\n.x\n@!\n");
        if (noise) {
            StaticTermCapture extra = {.formal = application_formal(caller_module, 0), .value = integer_type};
            const StaticTerm *unused;
            assert(static_term_nominal(&terms, caller_module, &extra, 1, (SourceSpan){0}, &unused));
        }
        StaticTermCapture child_input = {.formal = application_formal(target_module, 0), .value = integer_type};
        const StaticTerm *child;
        assert(static_term_nominal(&terms, target_module, &child_input, 1, (SourceSpan){0}, &child));
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = child};
        SourceArena destination;
        source_arena_init(&destination, sources.allocator);
        StaticKeyEncoding key = key_encode(&destination, key_ordinary(caller_module, &input, 1));
        assert(key.length < sizeof(previous));
        if (noise) { assert(child->ordinal != previous_ordinal && strcmp(previous, key.descriptor) == 0); }
        else { previous_ordinal = child->ordinal; memcpy(previous, key.descriptor, key.length + 1); }
        source_arena_destroy(&destination);
        destroy_guards();
    }
}

/** Two equal child values at every level must not flatten into a binary tree.
 * The key remains bounded by the number of distinct metadata nodes. This is
 * not a claim about runtime instance layout or executable admission.
 */
static void it_factors_deep_shared_nominal_dags_without_c_recursion(void) {
    const size_t depths[] = {0, 1, 2, 16, 256, 1024};
    for (size_t index = 0; index < sizeof(depths) / sizeof(*depths); index++) {
        setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\n",
            "@<\nfirst\n.a\nsecond\n.b\n@!\n");
        const StaticTerm *value = integer_type;
        for (size_t depth = 0; depth < depths[index]; depth++) {
            StaticTermCapture children[] = {{.formal = application_formal(target_module, 0), .value = value},
                {.formal = application_formal(target_module, 1), .value = value}};
            assert(static_term_nominal(&terms, target_module, children, 2, (SourceSpan){0}, &value));
        }
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = value};
        const StaticTermEnvironment *environment = key_ordinary(caller_module, &input, 1);
        StaticKeyPlan plan = {.terms = &terms, .environment = environment};
        source_arena_init(&plan.scratch, sources.allocator);
        assert(static_key_collect(&plan) == STATIC_KEY_OK && plan.count == depths[index] + 1);
        source_arena_destroy(&plan.scratch);
        SourceArena destination;
        source_arena_init(&destination, sources.allocator);
        StaticKeyEncoding key = key_encode(&destination, environment);
        assert(key.length < 64 * (depths[index] + 1) + 128);
        source_arena_destroy(&destination);
        destroy_guards();
    }
}

/** Execute source/solver staging, full-key distinctions and deep sharing. */
static void run(void) {
    it_preserves_named_partial_staging_and_solved_alias_identity();
    it_distinguishes_equal_arity_residuals_and_all_fixed_captures();
    it_keeps_full_residual_names_beyond_origin_capture_and_arity();
    it_preserves_root_kind_for_identical_module_basenames();
    it_ignores_unrelated_allocation_and_term_construction_order();
    it_factors_deep_shared_nominal_dags_without_c_recursion();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
