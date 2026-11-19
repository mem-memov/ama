/** @file
 * @brief Compose scoped static definitions with creation provenance, equality controls and key closure.
 */
#include "../common.c"

/** Exact original symbol identity includes its result/alias role and owner. */
static const StaticSymbol *named(const StaticModule *module, const char *name) {
    const StaticSymbol *symbol = static_symbol_find(module, static_name_site(module->definition, name));
    assert(symbol); return symbol;
}

/** Both pairs and first receipts retain their original defining expression. */
static const AstTypeExpression *definition_expression(const StaticSymbol *symbol) {
    if (symbol->role == STATIC_ALIAS) {
        assert(symbol->receiver && symbol->declaration->kind == AST_STATIC_PROJECTION);
        return symbol->declaration->as.static_projection;
    }
    assert(symbol->role == STATIC_RESULT && symbol->declaration->kind == AST_STATIC_RESULT);
    return symbol->declaration->as.static_result.expression;
}

/** Dependency locations belong to the parent's actual defining source. */
static SourceSpan result_site(const StaticSymbol *symbol) {
    return definition_expression(symbol)->span;
}

/** Results may reference themselves or another result, but their static
 * definitions cannot be expanded through a directed cycle.
 */
static void it_refuses_direct_and_mutual_active_static_result_definitions(void) {
    for (size_t mutual = 0; mutual < 2; mutual++) {
        setup_guards(mutual ? "@...\n...a\n...b\n...b\n...a\n...anchor\n..integer\n" :
            "@...\n...a\n...a\n...b\n..integer\n...anchor\n..integer\n", "@!\n");
        assert(caller_module->parameter_count == 0);
        const StaticCreationPath *root;
        assert(static_creation_root(&guards, caller_environment, &root));
        const StaticSymbol *a = named(caller_module, "...a"), *b = named(caller_module, "...b");
        const StaticDefinitionPath *first, *parent, *output = NULL;
        assert(static_definition_enter(&guards, NULL, root, a, (SourceSpan){0}, &first));
        parent = first;
        if (mutual) { assert(static_definition_enter(&guards, first, root, b, result_site(a), &parent)); }
        assert(!static_definition_enter(&guards, parent, root, a, result_site(mutual ? b : a), &output));
        assert(output == NULL && guards.status == STATIC_GUARD_CONFLICT);
        assert(guards.rejected_definition_path == parent && guards.repeated_definition_ancestor == first);
        assert(guards.rejected_definition->symbol == a && guards.creation_witness == NULL);
        assert(guards.definition_edge_count == mutual && guards.definition_path_count == mutual + 1);
        assert(diagnostic_valid(&guards.error) && solver.equation_count == 0);
        destroy_guards();
    }
}

/** Source-backed graph producer uses retain creation ancestry independently
 * of the definitions whose selected outputs they supply.
 */
static const StaticCreationPath *producer(const StaticCreationPath *parent, const AstTypeExpression *expression) {
    const StaticCreationPath *path;
    StaticCreationOutcome outcome;
    assert(static_creation_producer_enter(&guards, parent, expression, NULL, 0, &path, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED);
    return path;
}

/** These original source result definitions mutually read static producers'
 * static results. Previously cached out->a must still be checked when a new
 * root a requests out; the active definition ancestry alone has only a.
 */
static void it_rejects_cached_foreign_result_cycles_without_conflating_producer_creation_cycles(void) {
    setup_guards("@...\n.other\n ...out > ...a\n...anchor\n..integer\n",
        "@...\n.main\n ...a > ...out\n");
    assert(caller_module->parameter_count == 0 && target_module->parameter_count == 0);
    const StaticSymbol *a = named(caller_module, "...a"), *out = named(target_module, "...out");
    const StaticCreationPath *main_root;
    assert(static_creation_root(&guards, caller_environment, &main_root));
    const StaticCreationPath *other = producer(main_root, definition_expression(a));
    const StaticCreationPath *main_again = producer(other, definition_expression(out));
    assert(guards.vertex_count == 2 && guards.edge_count == 2 && main_again->vertex == main_root->vertex);
    const StaticDefinitionPath *out_root, *a_dependency, *a_root, *output = NULL;
    assert(static_definition_enter(&guards, NULL, other, out, (SourceSpan){0}, &out_root));
    assert(static_definition_enter(&guards, out_root, main_again, a, result_site(out), &a_dependency));
    assert(static_definition_enter(&guards, NULL, main_root, a, (SourceSpan){0}, &a_root));
    assert(a_dependency->node != a_root->node && a_dependency->node->representative == a_root->node->representative);
    assert(!static_definition_enter(&guards, a_root, other, out, result_site(a), &output));
    assert(output == NULL && guards.status == STATIC_GUARD_CONFLICT && guards.definition_witness);
    assert(guards.definition_closing_edge && guards.creation_witness == NULL);
    assert(guards.definition_edge_count == 2 && diagnostic_valid(&guards.error));
    assert(a->role == STATIC_ALIAS && a->owner == caller_module);
    destroy_guards();
}

/** A receiver alias is an original static definition in its caller scope. Its
 * edge to a foreign result is an acyclic dependency, not an owner parameter or
 * a cycle merely because the returned Type is already shared elsewhere.
 */
static void it_preserves_alias_definition_scope_and_idempotent_sibling_reads(void) {
    setup_guards("@...\n...a\n..integer\n.other\n ...out > ...alias\n",
        "@...\n...out\n..integer\n");
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const AstTypeExpression *expression = NULL;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_STATIC_PROJECTION) { expression = site->expression; break; }
    }
    assert(expression);
    const StaticCreationPath *other = producer(root, expression);
    const StaticSymbol *alias = named(caller_module, "...alias");
    const StaticDefinitionPath *first, *dependency, *again;
    assert(alias->role == STATIC_ALIAS && caller_module->parameter_count == 0);
    assert(static_definition_enter(&guards, NULL, root, alias, (SourceSpan){0}, &first));
    assert(static_definition_enter(&guards, first, other, named(target_module, "...out"), alias->site, &dependency));
    assert(first->node->symbol == alias && dependency->parent == first && dependency->creation == other);
    size_t before = allocation_count;
    assert(static_definition_enter(&guards, NULL, root, alias, (SourceSpan){0}, &again) && again == first);
    assert(static_definition_enter(&guards, first, other, named(target_module, "...out"), alias->site, &again));
    assert(again == dependency && allocation_count == before && guards.definition_edge_count == 1);
    destroy_guards();
}

/** Equality cycles do not imply static
 * definition edges. All owner formals, including unused u, close by exact aliases.
 */
static void it_keeps_anchored_equality_cycles_separate_from_static_definitions(void) {
    setup_guards("@...\n...a\n...t\n...unused\n...u\n...b\n...v\n...anchor\n..integer\n",
        "@!\n");
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const StaticTerm *t = application_symbol("...t"), *u = application_symbol("...u"), *v = application_symbol("...v");
    assert(static_constraint_equate(&solver, t, u, named(caller_module, "...t")->site));
    assert(static_constraint_equate(&solver, u, v, named(caller_module, "...u")->site));
    assert(static_constraint_equate(&solver, v, t, named(caller_module, "...v")->site));
    assert(static_constraint_equate(&solver, t, integer_type, named(caller_module, "...t")->site));
    const StaticDefinitionPath *a, *b;
    assert(static_definition_enter(&guards, NULL, root, named(caller_module, "...a"), (SourceSpan){0}, &a));
    assert(static_definition_enter(&guards, NULL, root, named(caller_module, "...b"), (SourceSpan){0}, &b));
    StaticGuardClosure closure;
    assert(static_guard_refresh(&guards, &closure) && closure == STATIC_GUARD_CLOSED);
    assert(a->node != b->node && guards.definition_edge_count == 0 && guards.edge_count == 0);
    assert(guards.definition_count == 2 && guards.status == STATIC_GUARD_OK);
    destroy_guards();
}

/** The operator receives a resolved scoped dependency from its later provider;
 * distinct owner keys initially have distinct a definitions. Equal-key closure
 * turns that retained dependency into a true self cycle, with no creation edge.
 */
static void it_rechecks_definition_edges_after_scoped_owners_become_equal(void) {
    setup_guards("@...\n...a\n...t\n...anchor\n..integer\n", "@!\n");
    const StaticSymbol *a = named(caller_module, "...a");
    const StaticCreationPath *generic, *concrete;
    assert(static_creation_root(&guards, caller_environment, &generic));
    StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, caller_module, &capture, 1, &environment));
    assert(static_creation_root(&guards, environment, &concrete));
    const StaticDefinitionPath *first, *second;
    assert(static_definition_enter(&guards, NULL, generic, a, (SourceSpan){0}, &first));
    assert(static_definition_enter(&guards, first, concrete, a, result_site(a), &second));
    assert(first->node != second->node && first->node->representative != second->node->representative);
    assert(static_constraint_equate(&solver, application_symbol("...t"), integer_type, a->site));
    StaticConstraintOutcome solved;
    assert(static_constraint_solve(&solver, &solved) && !solver.dirty);
    StaticGuardClosure closure = STATIC_GUARD_PENDING;
    assert(!static_guard_refresh(&guards, &closure));
    assert(closure == STATIC_GUARD_PENDING && guards.definition_witness && guards.creation_witness == NULL);
    assert(guards.edge_count == 0 && guards.definition_edge_count == 1);
    assert(first->node->representative == second->node->representative);
    destroy_guards();
}

/** Runtime parameters, copied handles and foreign source files cannot become
 * trusted definition dependencies by matching spelling or payload fields.
 */
static void it_refuses_foreign_symbols_paths_sites_and_missing_receivers(void) {
    for (size_t partition = 0; partition < 6; partition++) {
        setup_guards("@...\n...a\n...t\n...b\n..integer\n",
            "@...\n...out\n..integer\n");
        const StaticCreationPath *root;
        assert(static_creation_root(&guards, caller_environment, &root));
        const StaticSymbol *a = named(caller_module, "...a"), *b = named(caller_module, "...b");
        const StaticDefinitionPath *parent, *output = NULL;
        assert(static_definition_enter(&guards, NULL, root, a, (SourceSpan){0}, &parent));
        StaticSymbol copied_symbol = *b;
        StaticDefinitionPath copied_parent = *parent;
        StaticCreationPath copied_creation = *root;
        assert(!static_definition_enter(&guards, partition == 2 ? &copied_parent : parent,
            partition == 5 ? &copied_creation : root,
            partition == 0 ? &copied_symbol : partition == 1 ? named(caller_module, "...t") : b,
            partition == 3 ? named(target_module, "...out")->site : result_site(a),
            partition == 4 ? NULL : &output));
        assert(output == NULL && guards.status == STATIC_GUARD_INVALID && guards.definition_edge_count == 0);
        destroy_guards();
    }
}

/** Empty producer keys still settle queued equations. A previously closed
 * cached owner must not turn a new contradictory equation into a cache hit.
 */
static void it_checks_equations_for_zero_input_producers_before_roots_and_cached_reads(void) {
    for (size_t cached = 0; cached < 2; cached++) {
        setup_guards("@...\n...a\n..integer\n", "@!\n");
        const StaticCreationPath *root = NULL;
        const StaticDefinitionPath *definition;
        if (cached) {
            assert(static_creation_root(&guards, caller_environment, &root));
            assert(static_definition_enter(&guards, NULL, root, named(caller_module, "...a"),
                (SourceSpan){0}, &definition));
        }
        assert(static_constraint_equate(&solver, integer_type, target_constructor,
            named(caller_module, "...a")->site));
        StaticGuardClosure closure = STATIC_GUARD_PENDING;
        bool accepted = cached ? static_guard_refresh(&guards, &closure) :
            static_creation_root(&guards, caller_environment, &root);
        assert(!accepted && guards.status == STATIC_GUARD_CONFLICT);
        assert(cached || root == NULL);
        assert(closure == STATIC_GUARD_PENDING && guards.error.message == solver.error.message);
        destroy_guards();
    }
}

/** First receipts define aliases; later receipts constrain the original symbol
 * and cannot replace its expression or merge it with an equal-valued pair.
 */
static void it_preserves_first_receipt_definitions_across_later_equality_receipts(void) {
    setup_guards("@...\n.other\n ...out > ...alias\n.other\n ...out > ...alias\n...pair\n..integer\n",
        "@...\n...out\n..integer\n");
    const StaticSymbol *alias = named(caller_module, "...alias"), *pair = named(caller_module, "...pair");
    const StaticOccurrence *receipts[2];
    size_t count = 0;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver && layout_text_is(site->receiver->receiver, "...alias")) {
            assert(count < 2 && site->symbol == alias); receipts[count++] = site;
        }
    }
    assert(count == 2 && alias->receiver == receipts[0]->receiver && alias->receiver != receipts[1]->receiver);
    assert(alias->declaration == receipts[0]->statement && pair->role == STATIC_RESULT);
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const StaticCreationPath *first_producer = producer(root, receipts[0]->statement->as.static_projection);
    const StaticCreationPath *second_producer = producer(root, receipts[1]->statement->as.static_projection);
    assert(first_producer != second_producer && first_producer->vertex == second_producer->vertex);
    const StaticDefinitionPath *definition, *dependency, *again, *pair_definition;
    assert(static_definition_enter(&guards, NULL, root, alias, (SourceSpan){0}, &definition));
    assert(static_definition_enter(&guards, definition, first_producer, named(target_module, "...out"),
        result_site(alias), &dependency));
    const StaticTerm *alias_value, *pair_value;
    assert(static_term_symbol(&terms, caller_environment, alias, &alias_value));
    assert(static_term_symbol(&terms, caller_environment, pair, &pair_value));
    assert(static_constraint_equate(&solver, alias_value, integer_type, receipts[0]->site));
    assert(static_constraint_equate(&solver, alias_value, integer_type, receipts[1]->site));
    assert(static_constraint_equate(&solver, pair_value, integer_type, pair->site));
    assert(static_definition_enter(&guards, NULL, root, alias, (SourceSpan){0}, &again) && again == definition);
    assert(static_definition_enter(&guards, NULL, root, pair, (SourceSpan){0}, &pair_definition));
    assert(pair_definition->node != definition->node && guards.definition_count == 3);
    assert(guards.definition_edge_count == 1 && alias->declaration == receipts[0]->statement);
    assert(dependency->node->owner->environment->definition == target_module);
    destroy_guards();
}

/** Execute original-source cycles, scoped cache merging and unrelated-relation controls. */
static void run(void) {
    it_preserves_first_receipt_definitions_across_later_equality_receipts();
    it_checks_equations_for_zero_input_producers_before_roots_and_cached_reads();
    it_refuses_direct_and_mutual_active_static_result_definitions();
    it_rejects_cached_foreign_result_cycles_without_conflating_producer_creation_cycles();
    it_preserves_alias_definition_scope_and_idempotent_sibling_reads();
    it_keeps_anchored_equality_cycles_separate_from_static_definitions();
    it_rechecks_definition_edges_after_scoped_owners_become_equal();
    it_refuses_foreign_symbols_paths_sites_and_missing_receivers();
}
/** Return success after counted full teardown. */
int main(void) { run(); return 0; }
