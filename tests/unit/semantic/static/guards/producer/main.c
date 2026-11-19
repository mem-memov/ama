/** @file
 * @brief Check compiler-only graph owner maps, solver closure and definition cycles.
 */
#include "../common.c"

/** Generic graph roots retain scoped inputs; applying a full map changes only
 * original input reads. Equal source-order permutations share a compiler scope.
 */
static void it_preserves_producer_maps_without_manufacturing_nominal_types(void) {
    setup_guards("@...\n...copy\n...input\n...anchor\n..integer\n", "@!\n");
    assert(caller_constructor == NULL && caller_module->parameter_count == 1);
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    assert(root->owner->value == NULL && root->vertex == NULL);
    const StaticSymbol *formal = application_formal(caller_module, 0);
    const StaticTerm *variable;
    assert(static_term_symbol(&terms, caller_environment, formal, &variable));
    StaticTermCapture input = {.formal = formal, .value = integer_type};
    const StaticTermEnvironment *applied, *again;
    assert(static_term_producer_environment(&terms, caller_module, &input, 1, &applied));
    assert(applied != caller_environment && applied->owner_type == NULL && applied->producer_applied);
    size_t before = allocation_count;
    assert(static_term_producer_environment(&terms, caller_module, &input, 1, &again));
    assert(again == applied && allocation_count == before);
    const StaticTerm *read;
    assert(static_term_symbol(&terms, applied, formal, &read) && read == integer_type);
    assert(static_constraint_equate(&solver, variable, integer_type, formal->site));
    StaticGuardClosure closure;
    assert(static_guard_refresh(&guards, &closure) && closure == STATIC_GUARD_CLOSED);
    assert(root->vertex && root->vertex->value == NULL && root->vertex->key == applied);
    const StaticCreationPath *closed_root;
    assert(static_creation_root(&guards, applied, &closed_root));
    assert(closed_root->vertex == root->vertex);
    destroy_guards();
}

/** A graph with no inputs still has a distinct generic and explicit scope.
 * Actual static-definition self-dependency is rejected independently of an
 * otherwise finite complete producer key.
 */
static void it_rejects_definition_recursion_in_a_closed_static_producer(void) {
    setup_guards("@...\n...copy\n...copy\n...anchor\n..integer\n", "@!\n");
    assert(caller_module->parameter_count == 0);
    const StaticTermEnvironment *applied;
    assert(static_term_producer_environment(&terms, caller_module, NULL, 0, &applied));
    assert(applied != caller_environment && applied->producer_applied);
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, applied, &root) && root->vertex != NULL);
    const StaticSymbol *copy = static_symbol_find(caller_module, static_name_site(project.entry, "...copy"));
    assert(copy && copy->role == STATIC_RESULT);
    const StaticDefinitionPath *first, *output = NULL;
    assert(static_definition_enter(&guards, NULL, root, copy, (SourceSpan){0}, &first));
    assert(!static_definition_enter(&guards, first, root, copy,
        copy->declaration->as.static_result.expression->span, &output));
    assert(output == NULL && guards.status == STATIC_GUARD_CONFLICT);
    assert(guards.repeated_definition_ancestor == first);
    destroy_guards();
}

/** Refuse each map/guard normalization allocation with no leaked store records.
 */
static void it_unwinds_every_producer_map_and_root_preparation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_guards("@...\n...copy\n...input\n...anchor\n..integer\n", "@!\n");
            StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
            const StaticTermEnvironment *environment = NULL;
            const StaticCreationPath *root = NULL;
            size_t environments = terms.environment_count;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = phase ? static_creation_root(&guards, caller_environment, &root) :
                static_term_producer_environment(&terms, caller_module, &input, 1, &environment);
            if (!failure) { assert(success); total = allocation_count - before; assert(total); }
            else {
                assert(!success && environment == NULL && root == NULL);
                if (!phase) { assert(terms.environment_count == environments); }
                assert(terms.status == STATIC_TERM_OUT_OF_MEMORY || guards.status == STATIC_GUARD_OUT_OF_MEMORY);
            }
            failure_at = 0;
            destroy_guards();
        }
    }
}

/** A repeated producer must wait before preparing a new scope. Once solved,
 * identical keys close the creation cycle, while unequal full maps are refused.
 */
static void it_suspends_unknown_producer_recursion_and_checks_closed_keys(void) {
    for (size_t different = 0; different < 2; different++) {
        setup_guards("@...\n.main\n ...input < ...input\n ...copy > ...copy\n...anchor\n..integer\n", "@!\n");
        const StaticSymbol *formal = application_formal(caller_module, 0);
        const StaticTerm *variable;
        assert(static_term_symbol(&terms, caller_environment, formal, &variable));
        const StaticCreationPath *root, *output = NULL;
        assert(static_creation_root(&guards, caller_environment, &root));
        const AstTypeExpression *expression = NULL;
        for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
            if (site->origin == project.entry && site->receiver == NULL) { expression = site->expression; break; }
        }
        assert(expression);
        StaticTermCapture input = {.formal = formal, .value = different ? target_constructor : variable};
        size_t environments = terms.environment_count, paths = guards.path_count;
        StaticCreationOutcome outcome;
        assert(static_creation_producer_enter(&guards, root, expression, &input, 1, &output, &outcome));
        assert(outcome == STATIC_CREATION_PENDING && output == NULL);
        assert(terms.environment_count == environments && guards.path_count == paths);
        assert(static_constraint_equate(&solver, variable, integer_type, formal->site));
        bool entered = static_creation_producer_enter(&guards, root, expression, &input, 1, &output, &outcome);
        assert(entered == !different);
        if (different) {
            assert(output == NULL && guards.status == STATIC_GUARD_CONFLICT);
            assert(guards.earlier_creation_key != guards.rejected_creation_key);
            assert(guards.earlier_creation_value == NULL && guards.rejected_creation_value == NULL);
        } else {
            assert(outcome == STATIC_CREATION_ENTERED && output && output->parent == root);
            assert(output->vertex == root->vertex && guards.edge_count == 1);
        }
        destroy_guards();
    }
}

/** Exhaust a two-input map over concrete and symbolic values in both array
 * orders. Every input affects identity even if a consumer never reads it.
 */
static void it_interns_all_input_permutations_and_preserves_unused_values(void) {
    setup_guards("@...\n...first\n...a\n...second\n...b\n...anchor\n..integer\n", "@<\nvalue\n...x\n@!\n");
    const StaticTerm *symbol;
    assert(static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &symbol));
    const StaticTerm *values[] = {integer_type, target_constructor, symbol};
    const StaticTermEnvironment *identities[3][3];
    for (size_t left = 0; left < 3; left++) {
        for (size_t right = 0; right < 3; right++) {
            StaticTermCapture map[] = {
                {.formal = application_formal(caller_module, 0), .value = values[left]},
                {.formal = application_formal(caller_module, 1), .value = values[right]}};
            assert(static_term_producer_environment(&terms, caller_module, map, 2, &identities[left][right]));
            StaticTermCapture reverse[] = {map[1], map[0]};
            const StaticTermEnvironment *again;
            size_t before = allocation_count;
            assert(static_term_producer_environment(&terms, caller_module, reverse, 2, &again));
            assert(again == identities[left][right] && allocation_count == before);
            for (size_t a = 0; a <= left; a++) {
                for (size_t b = 0; b < 3; b++) {
                    if (a == left && b >= right) { break; }
                    assert(identities[a][b] != again);
                }
            }
        }
    }
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, identities[1][1], &root));
    assert(root->vertex && root->vertex->key == identities[1][1]);
    assert(root->vertex->key->producer_inputs->value->category == STATIC_CATEGORY_TEMPLATE);
    destroy_guards();
}

/** Bad maps fail before allocation/publication, including copied foreign
 * identities and duplicate keys. An ordinary constructor is never a producer.
 */
static void it_refuses_invalid_producer_maps_before_publication(void) {
    for (size_t partition = 0; partition < 9; partition++) {
        setup_guards("@...\n...first\n...a\n...second\n...b\n...anchor\n..integer\n", "@!\n");
        StaticSymbol copied = *application_formal(caller_module, 0);
        StaticTerm copied_value = *integer_type;
        StaticTermCapture map[] = {
            {.formal = application_formal(caller_module, 0), .value = integer_type},
            {.formal = application_formal(caller_module, 1), .value = integer_type}};
        if (partition == 0) { map[0].formal = &copied; }
        if (partition == 1) { map[0].value = &copied_value; }
        if (partition == 2) { map[1].formal = map[0].formal; }
        if (partition == 3) { map[1].value = NULL; }
        if (partition == 8) { terms.environment_count = SIZE_MAX; }
        size_t before = allocation_count, environments = terms.environment_count;
        const StaticTermEnvironment *output = NULL;
        assert(!static_term_producer_environment(&terms, partition == 4 ? target_module : caller_module,
            partition == 5 ? NULL : map, partition == 6 ? 1 : 2, partition == 7 ? NULL : &output));
        assert(output == NULL && terms.environment_count == environments && allocation_count == before);
        assert(terms.status == (partition == 8 ? STATIC_TERM_OUT_OF_MEMORY : STATIC_TERM_INVALID));
        destroy_guards();
    }
}

/** Actual lexical producer entry is failure-injected at every allocation,
 * including normalized owner identity, canonical vertices and reachability scratch.
 */
static void it_unwinds_every_source_backed_producer_entry_allocation_refusal(void) {
    for (size_t rejected = 0; rejected < 2; rejected++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_guards(rejected ?
                "@...\n.main\n ...input < ...input\n ...copy > ...copy\n...anchor\n..integer\n" :
                "@...\n.other\n ...input < ...local\n ...copy > ...received\n...anchor\n..integer\n",
                rejected ? "@!\n" : "@...\n...copy\n...input\n");
            const StaticModule *definition = rejected ? caller_module : target_module;
            const StaticTermEnvironment *environment = caller_environment;
            if (rejected) {
                StaticTermCapture fixed = {.formal = application_formal(caller_module, 0), .value = integer_type};
                assert(static_term_producer_environment(&terms, caller_module, &fixed, 1, &environment));
            }
            const StaticCreationPath *root, *output = NULL;
            assert(static_creation_root(&guards, environment, &root));
            const AstTypeExpression *expression = NULL;
            for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
                if (site->origin == definition->definition && site->receiver == NULL) { expression = site->expression; break; }
            }
            assert(expression);
            StaticTermCapture map = {.formal = application_formal(definition, 0),
                .value = rejected ? target_constructor : integer_type};
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            StaticCreationOutcome outcome;
            bool success = static_creation_producer_enter(&guards, root, expression, &map, 1, &output, &outcome);
            if (!failure) {
                assert(success == !rejected);
                if (rejected) { assert(output == NULL && guards.status == STATIC_GUARD_CONFLICT); }
                else { assert(output && outcome == STATIC_CREATION_ENTERED); }
                total = allocation_count - before;
                assert(total);
            } else { assert(!success && output == NULL && guards.status == STATIC_GUARD_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_guards();
        }
    }
}

/** Source ownership is checked by actual inventory occurrence, not copied AST
 * contents or an unregistered parent frame. Refusals publish no path or scope.
 */
static void it_refuses_foreign_producer_sites_and_receivers_without_allocation(void) {
    for (size_t partition = 0; partition < 4; partition++) {
        setup_guards("@...\n.other\n ...input < ...local\n ...copy > ...received\n...anchor\n..integer\n",
            "@...\n...copy\n...input\n");
        const StaticCreationPath *root, *output = NULL;
        assert(static_creation_root(&guards, caller_environment, &root));
        const AstTypeExpression *expression = caller_module->first_occurrence->expression;
        for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
            if (site->origin == target_module->definition && site->receiver == NULL) { expression = site->expression; break; }
        }
        AstTypeExpression copied = *expression;
        StaticCreationPath foreign = *root;
        StaticTermCapture map = {.formal = application_formal(target_module, 0), .value = integer_type};
        StaticCreationOutcome outcome;
        size_t before = allocation_count, environments = terms.environment_count, paths = guards.path_count;
        assert(!static_creation_producer_enter(&guards, partition == 0 ? &foreign : root,
            partition == 1 ? &copied : expression, &map, 1, partition == 2 ? NULL : &output,
            partition == 3 ? NULL : &outcome));
        assert(output == NULL && guards.status == STATIC_GUARD_INVALID);
        assert(allocation_count == before && terms.environment_count == environments && guards.path_count == paths);
        destroy_guards();
    }
}

/** Execute owner identity, independent cycle and all observed refusal cases. */
static void run(void) {
    it_refuses_foreign_producer_sites_and_receivers_without_allocation();
    it_interns_all_input_permutations_and_preserves_unused_values();
    it_refuses_invalid_producer_maps_before_publication();
    it_unwinds_every_source_backed_producer_entry_allocation_refusal();
    it_suspends_unknown_producer_recursion_and_checks_closed_keys();
    it_preserves_producer_maps_without_manufacturing_nominal_types();
    it_rejects_definition_recursion_in_a_closed_static_producer();
    it_unwinds_every_producer_map_and_root_preparation_refusal();
}
/** Return success only after counted cleanup. */
int main(void) { run(); return 0; }
