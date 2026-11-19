/** @file
 * @brief Check producer cache provenance, shared finite tails and delayed ordinary key closure.
 */
#include "../common.c"

/** Static producer edges have explicit named maps; the fixed edges make
 * concrete cached descendants independent of the current owner's unknown input.
 */
static const char *main_source = "@...\n...out\n...element\n"
    ".other\n ...element < ...element\n ...out > ...other result\n"
    ".other\n ...element < ..text\n ...out > ...fixed other result\n"
    ".leaf\n ...element < ..integer\n ...out > ...leaf result\n"
    ".leaf\n ...element < ..integer\n ...out > ...second leaf result\n"
    "...anchor\n..integer\n...text\n..text\n";
static const char *other_source = "@...\n...out\n...element\n"
    ".main\n ...element < ...element\n ...out > ...main result\n"
    ".leaf\n ...element < ..integer\n ...out > ...leaf result\n";
static const char *leaf_source = "@...\n...out\n...element\n"
    ".main\n ...element < ..text\n ...out > ...main result\n";

/** Load three actual graph origins and an independent concrete argument Type. */
static const StaticTerm *setup_producers(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/leaf", leaf_source);
    setup_guards(main_source, other_source);
    const StaticModule *text = static_module_find(&inventory, project_find(&project, "..text"));
    const StaticTerm *type;
    assert(text && static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &type));
    return type;
}

/** Select an exact written projection by its local defining receipt. */
static const AstTypeExpression *projection(const StaticModule *module, const char *receiver) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->receiver && layout_text_is(site->receiver->receiver, receiver)) {
            assert(site->statement->kind == AST_STATIC_PROJECTION);
            return site->statement->as.static_projection;
        }
    }
    assert(false); return NULL;
}

/** Bind the producer's full one-input identity before selecting a compilation root. */
static const StaticCreationPath *producer_root(const StaticModule *module, const StaticTerm *value) {
    StaticTermCapture input = {.formal = application_formal(module, 0), .value = value};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, module, &input, 1, &environment));
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, environment, &path));
    return path;
}

/** Inputs are resolved fixture IR for the selected source binding, not a
 * replacement for T211's graph equation evaluator. No output is evaluated here.
 */
static const StaticCreationPath *producer_child(const StaticCreationPath *parent,
    const char *receiver, const StaticModule *target, const StaticTerm *value) {
    StaticTermCapture input = {.formal = application_formal(target, 0), .value = value};
    const StaticCreationPath *path;
    StaticCreationOutcome outcome;
    assert(static_creation_producer_enter(&guards, parent,
        projection(parent->owner->environment->definition, receiver), &input, 1, &path, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED);
    return path;
}

/** A(integer)->B(integer)->Leaf(integer) and B(text)->A(text)->Leaf(integer)
 * share only the true finite tail. Source names alone must not join A/B keys.
 * These kernel checks do not admit unevaluated result definitions.
 */
static void it_preserves_distinct_producer_specializations_and_shared_finite_tails(void) {
    const StaticTerm *text = setup_producers();
    const StaticModule *leaf = static_module_find(&inventory, project_find(&project, ".leaf"));
    const StaticCreationPath *a = producer_root(caller_module, integer_type);
    const StaticCreationPath *b_child = producer_child(a, "...other result", target_module, integer_type);
    const StaticCreationPath *first_leaf = producer_child(b_child, "...leaf result", leaf, integer_type);
    const StaticCreationPath *b = producer_root(target_module, text);
    const StaticCreationPath *a_child = producer_child(b, "...main result", caller_module, text);
    const StaticCreationPath *second_leaf = producer_child(a_child, "...leaf result", leaf, integer_type);
    assert(a->vertex != a_child->vertex && b->vertex != b_child->vertex);
    assert(first_leaf->vertex == second_leaf->vertex && first_leaf != second_leaf);
    assert(guards.vertex_count == 5 && guards.edge_count == 4 && guards.path_count == 6);
    assert(first_leaf->edge != second_leaf->edge && first_leaf->site.source != second_leaf->site.source);
    size_t before = allocation_count, live = live_allocations;
    for (size_t repeat = 0; repeat < 100; repeat++) {
        StaticGuardClosure closure;
        assert(static_guard_refresh(&guards, &closure) && closure == STATIC_GUARD_CLOSED);
    }
    assert(allocation_count == before && live_allocations == live);
    destroy_guards();
}

/** A new Leaf(text)->cached A(text) edge must inspect A's existing Leaf(integer)
 * descendant. The immediate target alone has a different origin from the root.
 */
static void it_rechecks_cached_descendants_before_publishing_a_new_creation(void) {
    for (size_t root_first = 0; root_first < 2; root_first++) {
        const StaticTerm *text = setup_producers();
        const StaticModule *leaf = static_module_find(&inventory, project_find(&project, ".leaf"));
        const StaticCreationPath *leaf_root = root_first ? producer_root(leaf, text) : NULL;
        const StaticCreationPath *b = producer_root(target_module, text);
        const StaticCreationPath *a = producer_child(b, "...main result", caller_module, text);
        (void)producer_child(a, "...leaf result", leaf, integer_type);
        if (!root_first) { leaf_root = producer_root(leaf, text); }
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = text};
        const StaticCreationPath *output = NULL;
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        assert(!static_creation_producer_enter(&guards, leaf_root, projection(leaf, "...main result"),
            &input, 1, &output, &outcome));
        assert(output == NULL && outcome == STATIC_CREATION_PENDING && guards.status == STATIC_GUARD_CONFLICT);
        assert(guards.creation_witness && guards.closing_edge && guards.vertex_count == 4 && guards.edge_count == 3);
        assert(guards.creation_witness->parent && diagnostic_valid(&guards.error));
        assert(guards.earlier_creation_key != guards.rejected_creation_key);
        destroy_guards();
    }
}

/** Distinct written receipts under one parent retain distinct creation uses
 * even when their target producer key is identical and already cached.
 */
static void it_keeps_separate_source_uses_of_the_same_producer_key(void) {
    (void)setup_producers();
    const StaticModule *leaf = static_module_find(&inventory, project_find(&project, ".leaf"));
    const StaticCreationPath *a = producer_root(caller_module, integer_type);
    const StaticCreationPath *first = producer_child(a, "...leaf result", leaf, integer_type);
    const StaticCreationPath *second = producer_child(a, "...second leaf result", leaf, integer_type);
    assert(first != second && first->vertex == second->vertex && first->edge != second->edge);
    size_t live = live_allocations;
    assert(producer_child(a, "...leaf result", leaf, integer_type) == first);
    assert(producer_child(a, "...second leaf result", leaf, integer_type) == second);
    assert(live_allocations == live && guards.path_count == 3 && guards.edge_count == 2);
    destroy_guards();
}

/** Ordinary field inference prepares new field-local input holes. Their exact
 * source ownership is independent of the owner's own parameter variables.
 */
static const AstTypeExpression *field(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Enter one source-backed ordinary field with independently inferred inputs. */
static const StaticCreationPath *symbolic_child(const StaticCreationPath *parent, const char *name, const StaticTerm *constructor) {
    StaticApplicationContext context;
    assert(static_application_context(&applications, parent->owner->environment,
        field(parent->owner->environment->definition, name), &context));
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, constructor, NULL, 0, context, &record));
    const StaticCreationPath *path;
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, parent, record, &path, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED);
    return path;
}

/** Independent scopes can later merge into a permitted same-key A/B cycle or
 * a cached path between unequal A keys. External solving cannot hide new edges.
 */
static void it_rechecks_all_incoming_provenance_when_symbolic_keys_close(void) {
    for (size_t unequal = 0; unequal < 2; unequal++) {
        setup_guards("@<\nfirst\n...a\nsecond\n...b\n@!\nanchor&\n..integer\nchild&\n.other\n",
            "@<\nfirst\n...a\nsecond\n...b\n@!\nback&\n.main\n");
        const StaticCreationPath *a, *b;
        assert(static_creation_root(&guards, caller_environment, &a));
        (void)symbolic_child(a, "child", target_constructor);
        const StaticTermEnvironment *b_environment;
        assert(static_term_environment(&terms, target_module, NULL, &b_environment));
        assert(static_creation_root(&guards, b_environment, &b));
        const StaticCreationPath *returned_a = symbolic_child(b, "back", caller_constructor);
        assert(guards.path_count == 4 && guards.vertex_count == 0);
        StaticTermCapture map[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}};
        const StaticTerm *other;
        assert(static_term_nominal(&terms, target_module, map, 2, (SourceSpan){0}, &other));
        for (const StaticCreationPath *path = guards.first_path; path; path = path->next) {
            for (const StaticTermCapture *capture = path->owner->value->as.nominal.first_capture; capture; capture = capture->next) {
                const StaticTerm *argument = unequal && path == returned_a && capture->formal == application_formal(caller_module, 0)
                    ? other : integer_type;
                assert(static_constraint_equate(&solver, capture->value, argument, capture->formal->site));
            }
        }
        StaticConstraintOutcome solved;
        assert(static_constraint_solve(&solver, &solved) && !solver.dirty);
        StaticGuardClosure closure = STATIC_GUARD_PENDING;
        bool accepted = static_guard_refresh(&guards, &closure);
        assert(accepted == !unequal);
        if (!unequal) {
            assert(closure == STATIC_GUARD_CLOSED && guards.vertex_count == 2 && guards.edge_count == 2);
        } else {
            assert(closure == STATIC_GUARD_PENDING && guards.status == STATIC_GUARD_CONFLICT);
            assert(guards.creation_witness && guards.rejected_creation_key != guards.earlier_creation_key);
        }
        assert(a->owner->environment == caller_environment && a->owner->value != a->vertex->value);
        destroy_guards();
    }
}

/** A symbolic incoming A can reach a complete cached B->A->Leaf path. Its
 * unknown key cannot prove a conflict; later independent evidence decides it.
 */
static void it_checks_cached_descendants_against_a_still_symbolic_ancestor(void) {
    for (size_t unequal = 0; unequal < 2; unequal++) {
        const StaticTerm *text = setup_producers();
        const StaticModule *leaf = static_module_find(&inventory, project_find(&project, ".leaf"));
        const StaticCreationPath *b = producer_root(target_module, text);
        const StaticCreationPath *a = producer_child(b, "...main result", caller_module, text);
        (void)producer_child(a, "...leaf result", leaf, integer_type);
        const StaticCreationPath *symbolic;
        assert(static_creation_root(&guards, caller_environment, &symbolic) && symbolic->vertex == NULL);
        const StaticCreationPath *incoming = producer_child(symbolic, "...fixed other result", target_module, text);
        assert(incoming->vertex == b->vertex);
        StaticGuardClosure closure;
        assert(static_guard_refresh(&guards, &closure) && closure == STATIC_GUARD_PENDING);
        assert(guards.status == STATIC_GUARD_OK && symbolic->vertex == NULL && guards.creation_witness == NULL);
        assert(static_constraint_equate(&solver, application_symbol("...element"), unequal ? integer_type : text,
            application_formal(caller_module, 0)->site));
        bool accepted = static_guard_refresh(&guards, &closure);
        assert(accepted == !unequal);
        if (unequal) { assert(guards.creation_witness && guards.rejected_creation_key != guards.earlier_creation_key); }
        else { assert(closure == STATIC_GUARD_CLOSED && symbolic->vertex == a->vertex); }
        destroy_guards();
    }
}

/** Known B retains an unknown ordinary A field, while another unknown A root
 * requires B. Neither unresolved A is guessed equal or unequal to the other.
 */
static void it_checks_cached_symbolic_descendants_without_publishing_concrete_edges(void) {
    setup_guards("@<\nfirst\n...a\nsecond\n...b\n@!\nanchor&\n..integer\nfixed&\n.other\n",
        "@<\nfirst\n...a\nsecond\n...b\n@!\ndeferred&\n.main\n");
    StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
        {.formal = application_formal(target_module, 1), .value = integer_type}};
    const StaticTerm *value;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &value));
    assert(static_term_environment(&terms, target_module, value, &environment));
    const StaticCreationPath *b;
    assert(static_creation_root(&guards, environment, &b));
    const StaticCreationPath *deferred = symbolic_child(b, "deferred", caller_constructor);
    const StaticCreationPath *symbolic;
    assert(static_creation_root(&guards, caller_environment, &symbolic));
    StaticApplicationContext context;
    assert(static_application_context(&applications, caller_environment, field(caller_module, "fixed"), &context));
    StaticApplicationInput inputs[] = {{.formal = captures[0].formal, .value = integer_type},
        {.formal = captures[1].formal, .value = integer_type}};
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, target_constructor, inputs, 2, context, &record));
    const StaticCreationPath *output = NULL;
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, symbolic, record, &output, &outcome));
    assert(output && outcome == STATIC_CREATION_ENTERED && output->vertex == b->vertex);
    StaticGuardClosure closure;
    assert(static_guard_refresh(&guards, &closure) && closure == STATIC_GUARD_PENDING);
    assert(guards.status == STATIC_GUARD_OK && guards.creation_witness == NULL);
    assert(symbolic->vertex == NULL && deferred->vertex == NULL && guards.vertex_count == 1 && guards.edge_count == 0);
    destroy_guards();
}

/** Execute finite paths, distinct original uses and delayed cache-key merging. */
static void run(void) {
    it_preserves_distinct_producer_specializations_and_shared_finite_tails();
    it_rechecks_cached_descendants_before_publishing_a_new_creation();
    it_keeps_separate_source_uses_of_the_same_producer_key();
    it_rechecks_all_incoming_provenance_when_symbolic_keys_close();
    it_checks_cached_descendants_against_a_still_symbolic_ancestor();
    it_checks_cached_symbolic_descendants_without_publishing_concrete_edges();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
