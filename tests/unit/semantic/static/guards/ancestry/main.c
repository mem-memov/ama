/** @file
 * @brief Check pending recurrence, exact closed cycles and unequal keys separately from finite nesting.
 */
#include "../common.c"

/** Distinct source applications cover unchanged, permuted and growing captures. */
static const char *caller_source = "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\nanchor&\n..integer\nchild&\n.other\n"
    "direct&\n.main\nsame&\n.main\npermuted&\n.main\ngrowing&\n.main\nnested&\n.other\n";
static const char *target_source = "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\nback&\n.main\n";

/** Registered declaration syntax determines the lexical owner, independently of
 * whichever constructor is supplied through the prepared head.
 */
static const AstTypeExpression *field(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_STATIC_RESULT &&
            layout_text_is(site->statement->as.static_result.name, name)) { return site->expression; }
        if (site->root && site->receiver == NULL && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Prepare one exact context-backed structural application; static definitions
 * and runtime fields are never expanded by the fixture.
 */
static const StaticApplicationRecord *apply(const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, const StaticTerm *head, const StaticApplicationInput *inputs, size_t count) {
    StaticApplicationContext context;
    assert(static_application_context(&applications, environment, expression, &context));
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, head, inputs, count, context, &record));
    return record;
}

/** Unknown repeated keys cannot prove equality or inequality. None of these
 * requests may prepare another scope or publish a path before real evidence.
 */
static void it_suspends_unresolved_equal_permuted_and_growing_requests(void) {
    const char *names[] = {"direct", "same", "permuted", "growing"};
    for (size_t mode = 0; mode < 4; mode++) {
        setup_guards(caller_source, target_source);
        const StaticCreationPath *root;
        assert(static_creation_root(&guards, caller_environment, &root));
        const StaticTerm *a = application_symbol("...a"), *b = application_symbol("...b");
        StaticApplicationInput inputs[] = {{.formal = application_formal(caller_module, 0), .value = a},
            {.formal = application_formal(caller_module, 1), .value = b}};
        if (mode == 2) { inputs[0].value = b; inputs[1].value = a; }
        if (mode == 3) {
            StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = a},
                {.formal = application_formal(target_module, 1), .value = b}};
            assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &inputs[0].value));
        }
        const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, names[mode]),
            caller_constructor, mode ? inputs : NULL, mode ? 2 : 0);
        size_t environments = terms.environment_count;
        const StaticCreationPath *output = root;
        StaticCreationOutcome outcome = STATIC_CREATION_ENTERED;
        assert(static_creation_enter(&guards, root, record, &output, &outcome));
        assert(output == root && outcome == STATIC_CREATION_PENDING && guards.path_count == 1);
        assert(guards.status == STATIC_GUARD_OK && terms.environment_count == environments);
        assert(guards.rejected_origin == NULL && guards.error.related_count == 0);
        destroy_guards();
    }
}

/** The first distinct template may be inspected symbolically; returning to A
 * waits rather than expanding a second unknown A or asserting a false conflict.
 */
static void it_suspends_mutual_symbolic_recurrence_without_an_extra_scope(void) {
    setup_guards(caller_source, target_source);
    const StaticCreationPath *root, *child, *output = NULL;
    assert(static_creation_root(&guards, caller_environment, &root));
    const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, "child"), target_constructor, NULL, 0);
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, root, record, &child, &outcome) && outcome == STATIC_CREATION_ENTERED);
    const StaticApplicationRecord *back = apply(child->owner->environment, field(target_module, "back"), caller_constructor, NULL, 0);
    size_t environments = terms.environment_count;
    assert(static_creation_enter(&guards, child, back, &output, &outcome));
    assert(output == NULL && outcome == STATIC_CREATION_PENDING && guards.path_count == 2);
    assert(guards.status == STATIC_GUARD_OK && terms.environment_count == environments);
    destroy_guards();
}

/** Full unused argument maps count: equality closes the key, but permutation
 * of distinct Types and growth to another concrete Type are exact conflicts.
 */
static void it_closes_identical_keys_and_rejects_closed_permutation_or_growth(void) {
    for (size_t mode = 0; mode < 3; mode++) {
        setup_guards(caller_source, target_source);
        const StaticTerm *a = application_symbol("...a"), *b = application_symbol("...b");
        StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}};
        const StaticTerm *other;
        assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &other));
        assert(static_constraint_equate(&solver, a, integer_type, (SourceSpan){0}));
        assert(static_constraint_equate(&solver, b, mode == 1 ? other : integer_type, (SourceSpan){0}));
        const StaticCreationPath *root, *output = NULL;
        assert(static_creation_root(&guards, caller_environment, &root) && root->vertex);
        StaticApplicationInput inputs[] = {{.formal = application_formal(caller_module, 0), .value = a},
            {.formal = application_formal(caller_module, 1), .value = b}};
        const char *name = "same";
        if (mode == 1) { name = "permuted"; inputs[0].value = b; inputs[1].value = a; }
        if (mode == 2) { name = "growing"; inputs[0].value = other; }
        const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, name), caller_constructor, inputs, 2);
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        bool accepted = static_creation_enter(&guards, root, record, &output, &outcome);
        assert(accepted == (mode == 0));
        if (mode == 0) {
            assert(output && outcome == STATIC_CREATION_ENTERED && output->vertex == root->vertex);
            assert(guards.vertex_count == 1 && guards.edge_count == 1 && guards.status == STATIC_GUARD_OK);
        } else {
            assert(output == NULL && outcome == STATIC_CREATION_PENDING && guards.status == STATIC_GUARD_CONFLICT);
            assert(guards.repeated_ancestor == root && guards.rejected_path == root);
            assert(guards.earlier_creation_value == root->vertex->value);
            assert(guards.rejected_creation_value != guards.earlier_creation_value);
            assert(guards.error.related_count == 2 && diagnostic_valid(&guards.error));
        }
        destroy_guards();
    }
}

/** A known field Type alone cannot choose unknown owner parameters. Real
 * later owner evidence makes the same deferred request either equal or unequal.
 */
static void it_retries_pending_comparisons_only_after_independent_owner_evidence(void) {
    for (size_t unequal = 0; unequal < 2; unequal++) {
        setup_guards(caller_source, target_source);
        const StaticCreationPath *root, *output = NULL;
        assert(static_creation_root(&guards, caller_environment, &root));
        StaticApplicationInput inputs[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
            {.formal = application_formal(caller_module, 1), .value = integer_type}};
        const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, "direct"), caller_constructor, inputs, 2);
        StaticCreationOutcome outcome;
        size_t environments = terms.environment_count;
        assert(static_creation_enter(&guards, root, record, &output, &outcome));
        assert(outcome == STATIC_CREATION_PENDING && output == NULL && terms.environment_count == environments);
        assert(static_creation_enter(&guards, root, record, &output, &outcome));
        assert(outcome == STATIC_CREATION_PENDING && guards.path_count == 1 && terms.environment_count == environments);
        StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}};
        const StaticTerm *other;
        assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &other));
        assert(static_constraint_equate(&solver, application_symbol("...a"), integer_type, (SourceSpan){0}));
        assert(static_constraint_equate(&solver, application_symbol("...b"), unequal ? other : integer_type, (SourceSpan){0}));
        bool accepted = static_creation_enter(&guards, root, record, &output, &outcome);
        assert(accepted == !unequal);
        if (!unequal) { assert(outcome == STATIC_CREATION_ENTERED && output->vertex == root->vertex); }
        else { assert(outcome == STATIC_CREATION_PENDING && output == NULL && guards.status == STATIC_GUARD_CONFLICT); }
        destroy_guards();
    }
}

/** Closed A/B keys can reference each other through scalar references or
 * initially empty owning lists; graph checking never constructs their values.
 */
static void it_closes_mutual_reference_and_empty_list_keys(void) {
    for (size_t lists = 0; lists < 2; lists++) {
        const char *a_source = lists ? "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\nanchor&\n..integer\nchild[]\n.other\n" : caller_source;
        const char *b_source = lists ? "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\nback[]\n.main\n" : target_source;
        setup_guards(a_source, b_source);
        StaticTermCapture captures[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
            {.formal = application_formal(caller_module, 1), .value = integer_type}};
        const StaticTerm *value;
        const StaticTermEnvironment *environment;
        assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &value));
        assert(static_term_environment(&terms, caller_module, value, &environment));
        const StaticCreationPath *root, *child, *back;
        assert(static_creation_root(&guards, environment, &root));
        StaticApplicationInput inputs[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}};
        const StaticApplicationRecord *forward = apply(environment, field(caller_module, "child"), target_constructor, inputs, 2);
        StaticCreationOutcome outcome;
        assert(static_creation_enter(&guards, root, forward, &child, &outcome) && outcome == STATIC_CREATION_ENTERED);
        inputs[0].formal = application_formal(caller_module, 0);
        inputs[1].formal = application_formal(caller_module, 1);
        const StaticApplicationRecord *reverse = apply(child->owner->environment, field(target_module, "back"), caller_constructor, inputs, 2);
        assert(static_creation_enter(&guards, child, reverse, &back, &outcome) && outcome == STATIC_CREATION_ENTERED);
        assert(back->vertex == root->vertex && back->owner->environment == environment);
        assert(guards.vertex_count == 2 && guards.edge_count == 2 && guards.status == STATIC_GUARD_OK);
        destroy_guards();
    }
}

/** Sweep the actual same-key comparison, normalization and edge publication
 * allocations. Refusal retains no successful receiver and all ownership unwinds.
 */
static void it_unwinds_every_closed_repeat_entry_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_guards(caller_source, target_source);
        assert(static_constraint_equate(&solver, application_symbol("...a"), integer_type, (SourceSpan){0}));
        assert(static_constraint_equate(&solver, application_symbol("...b"), integer_type, (SourceSpan){0}));
        const StaticCreationPath *root, *output = NULL;
        assert(static_creation_root(&guards, caller_environment, &root));
        StaticApplicationInput inputs[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
            {.formal = application_formal(caller_module, 1), .value = integer_type}};
        const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, "same"), caller_constructor, inputs, 2);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        bool success = static_creation_enter(&guards, root, record, &output, &outcome);
        if (!cut) { assert(success && output->vertex == root->vertex); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && output == NULL && outcome == STATIC_CREATION_PENDING && guards.status == STATIC_GUARD_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_guards();
    }
}

/** Both written applications originate in main. Outer result capture structure
 * cannot create a false outer-to-inner creation edge or ban finite repeated nesting.
 */
static void it_accepts_finite_nested_and_sibling_applications_of_one_origin(void) {
    setup_guards("@...\n...nested\n.other\n ...a < .other\n  ...a < ..integer\n  ...b < ..integer\n ...b < ..integer\n...anchor\n..integer\n",
        "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\n");
    const StaticCreationPath *root, *outer, *inner;
    assert(static_creation_root(&guards, caller_environment, &root));
    const AstTypeExpression *outer_expression = field(caller_module, "...nested"), *inner_expression = NULL;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (!site->root && site->receiver == NULL && site->statement && site->statement->kind == AST_STATIC_RESULT &&
            layout_text_is(site->statement->as.static_result.name, "...nested") &&
            layout_text_is(site->expression->head, ".other")) { inner_expression = site->expression; break; }
    }
    assert(inner_expression);
    StaticApplicationInput inputs[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
        {.formal = application_formal(target_module, 1), .value = integer_type}};
    const StaticApplicationRecord *inner_record = apply(caller_environment, inner_expression, target_constructor, inputs, 2);
    inputs[0].value = inner_record->value;
    const StaticApplicationRecord *outer_record = apply(caller_environment, outer_expression, target_constructor, inputs, 2);
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, root, outer_record, &outer, &outcome) && outcome == STATIC_CREATION_ENTERED);
    assert(static_creation_enter(&guards, root, inner_record, &inner, &outcome) && outcome == STATIC_CREATION_ENTERED);
    assert(inner->parent == root && outer->parent == root && inner->owner != outer->owner);
    assert(inner->owner->environment->definition == outer->owner->environment->definition);
    const StaticCreationPath *again;
    size_t before = allocation_count;
    assert(static_creation_enter(&guards, root, inner_record, &again, &outcome));
    assert(again == inner && guards.path_count == 3 && allocation_count == before);
    assert(static_creation_root(&guards, caller_environment, &again) && again == root);
    destroy_guards();
}

/** A syntactically nested child retains main's lexical environment. Supplying
 * the outer result as parent is invalid internal provenance, not a language cycle.
 */
static void it_refuses_foreign_or_incorrect_lexical_parent_handles(void) {
    for (size_t copied = 0; copied < 2; copied++) {
        setup_guards(caller_source, target_source);
        const StaticCreationPath *root, *child, *output = NULL;
        assert(static_creation_root(&guards, caller_environment, &root));
        const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, "child"), target_constructor, NULL, 0);
        StaticCreationOutcome outcome;
        assert(static_creation_enter(&guards, root, record, &child, &outcome));
        StaticCreationPath foreign = *root;
        assert(!static_creation_enter(&guards, copied ? &foreign : child, record, &output, &outcome));
        assert(output == NULL && guards.status == STATIC_GUARD_INVALID && guards.path_count == 2);
        destroy_guards();
    }
}

/** Execute symbolic recurrence, finite nesting and exact lexical provenance cases. */
static void run(void) {
    it_suspends_unresolved_equal_permuted_and_growing_requests();
    it_suspends_mutual_symbolic_recurrence_without_an_extra_scope();
    it_closes_identical_keys_and_rejects_closed_permutation_or_growth();
    it_retries_pending_comparisons_only_after_independent_owner_evidence();
    it_closes_mutual_reference_and_empty_list_keys();
    it_unwinds_every_closed_repeat_entry_allocation_refusal();
    it_accepts_finite_nested_and_sibling_applications_of_one_origin();
    it_refuses_foreign_or_incorrect_lexical_parent_handles();
}
/** Return success after counted guard/application/source teardown. */
int main(void) { run(); return 0; }
