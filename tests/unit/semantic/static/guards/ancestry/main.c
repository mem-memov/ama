/** @file
 * @brief Check direct and mutual symbolic recurrence separately from finite supplied nesting.
 */
#include "../common.c"

/** Distinct source applications cover unchanged, permuted and growing captures. */
static const char *caller_source = "@<\n...a\n...b\n@!\nanchor&\n..integer\nchild&\n.other\n"
    "direct&\n.main\nsame&\n.main\n ...a < ...a\n ...b < ...b\n"
    "permuted&\n.main\n ...a < ...b\n ...b < ...a\n"
    "growing&\n.main\n ...a < .other\n  ...a < ...a\n  ...b < ...b\n ...b < ...b\n"
    "nested&\n.other\n ...a < .other\n  ...a < ..integer\n  ...b < ..integer\n ...b < ..integer\n";
static const char *target_source = "@<\n...a\n...b\n@!\nback&\n.main\n";

/** Registered declaration syntax determines the lexical owner, independently of
 * whichever constructor is supplied through the prepared head.
 */
static const AstTypeExpression *field(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
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

/** The root's source already appears in every self creation, regardless of
 * whether its key is equal, permuted, growing or still contains inference holes.
 */
static void it_rejects_direct_equal_permuted_and_growing_origins_before_expansion(void) {
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
        const StaticCreationPath *output = NULL;
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        assert(!static_creation_enter(&guards, root, record, &output, &outcome));
        assert(output == NULL && outcome == STATIC_CREATION_PENDING && guards.path_count == 1);
        assert(guards.status == STATIC_GUARD_CONFLICT && guards.repeated_ancestor == root);
        assert(guards.rejected_path == root && guards.rejected_origin == caller_module);
        assert(guards.error.primary.span.start == record->context.expression->span.start);
        assert(guards.error.related_count == 2 && diagnostic_valid(&guards.error));
        destroy_guards();
    }
}

/** Mutual recurrence is rejected with its whole immutable path, including the
 * accepted intermediate creation and original root, before following the back edge.
 */
static void it_rejects_mutual_symbolic_origins_and_retains_each_original_use(void) {
    setup_guards(caller_source, target_source);
    const StaticCreationPath *root, *child, *output = NULL;
    assert(static_creation_root(&guards, caller_environment, &root));
    const StaticApplicationRecord *record = apply(caller_environment, field(caller_module, "child"), target_constructor, NULL, 0);
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, root, record, &child, &outcome) && outcome == STATIC_CREATION_ENTERED);
    assert(child->parent == root && child->application == record && !child->owner->value->ground);
    const StaticApplicationRecord *back = apply(child->owner->environment, field(target_module, "back"), caller_constructor, NULL, 0);
    assert(!static_creation_enter(&guards, child, back, &output, &outcome));
    assert(output == NULL && guards.status == STATIC_GUARD_CONFLICT && guards.path_count == 2);
    assert(guards.rejected_path == child && guards.repeated_ancestor == root);
    assert(guards.rejected_path->application->context.expression == field(caller_module, "child"));
    assert(guards.rejected_site.start == field(target_module, "back")->span.start);
    destroy_guards();
}

/** Both written applications originate in main. Outer result capture structure
 * cannot create a false outer-to-inner creation edge or ban finite repeated nesting.
 */
static void it_accepts_finite_nested_and_sibling_applications_of_one_origin(void) {
    setup_guards(caller_source, target_source);
    const StaticCreationPath *root, *outer, *inner;
    assert(static_creation_root(&guards, caller_environment, &root));
    const AstTypeExpression *outer_expression = field(caller_module, "nested"), *inner_expression = NULL;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (!site->root && site->receiver == NULL && site->statement && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, "nested") &&
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
    it_rejects_direct_equal_permuted_and_growing_origins_before_expansion();
    it_rejects_mutual_symbolic_origins_and_retains_each_original_use();
    it_accepts_finite_nested_and_sibling_applications_of_one_origin();
    it_refuses_foreign_or_incorrect_lexical_parent_handles();
}
/** Return success after counted guard/application/source teardown. */
int main(void) { run(); return 0; }
