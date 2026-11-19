/** @file
 * @brief Verify canonical local Type equations, unknown owners and distinct source operations.
 */
#include "../common.c"

/** Canonical owner sharing links local compile-time holes without conflating
 * distinct declarations; applied formals read the supplied concrete Type.
 */
static void it_canonicalizes_equal_scopes_and_preserves_local_occurrences(void) {
    setup_registry("@<\nvalue\n...type\n@!\nfirst&\n..integer\nsecond&\n..integer\n", "@!\n");
    const StaticSymbol *formal = application_formal(caller_module, 0);
    const StaticTerm *parameter;
    assert(static_term_symbol(&terms, caller_environment, formal, &parameter));
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, caller_environment, &path));
    StaticRegistryUse *use;
    StaticRegistryKeyState state;
    assert(static_registry_use(&registry, path, &use, &state));
    const StaticTerm *canonical = NULL;
    assert(static_registry_scope_variable(&registry, use, parameter, &canonical, &state));
    assert(state == STATIC_REGISTRY_KEY_PENDING && canonical == NULL);
    assert(static_constraint_equate(&solver, parameter, integer_type, formal->site));
    assert(static_registry_scope_variable(&registry, use, parameter, &canonical, &state));
    assert(state == STATIC_REGISTRY_KEY_CLOSED && canonical == integer_type);
    const StaticOccurrence *first = caller_module->first_occurrence;
    while (first && (!first->statement || first->statement->kind != AST_DECLARATION)) { first = first->next; }
    assert(first);
    const StaticOccurrence *second = first->next;
    while (second && (!second->statement || second->statement == first->statement ||
        second->statement->kind != AST_DECLARATION)) { second = second->next; }
    assert(second);
    const StaticTerm *original, *other, *shared, *direct;
    assert(static_term_inference(&terms, caller_environment, formal, first->statement, NULL, &original));
    assert(static_term_inference(&terms, caller_environment, formal, second->statement, NULL, &other));
    assert(static_registry_scope_variable(&registry, use, original, &shared, &state));
    assert(static_term_inference(&terms, use->candidate->key, formal, first->statement, NULL, &direct));
    assert(shared == direct && shared != original);
    const StaticTerm *different;
    assert(static_registry_scope_variable(&registry, use, other, &different, &state));
    assert(different != shared);
    assert(static_constraint_equate(&solver, original, integer_type, original->site));
    const StaticTerm *normalized;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, shared, &normalized, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && normalized == integer_type);
    assert(use->candidate->state == STATIC_CANDIDATE_NEW);
    destroy_registry();
}

/** Every canonical-hole or equation allocation refusal remains sticky and owned;
 * the original input and public receivers remain valid and unchanged.
 */
static void it_releases_every_partial_scope_preparation_on_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_registry("@<\nvalue\n...type\n@!\nanchor&\n..integer\n", "@!\n");
        const StaticSymbol *formal = application_formal(caller_module, 0);
        const StaticTerm *parameter;
        assert(static_term_symbol(&terms, caller_environment, formal, &parameter));
        assert(static_constraint_equate(&solver, parameter, integer_type, formal->site));
        const StaticCreationPath *path;
        assert(static_creation_root(&guards, caller_environment, &path));
        StaticRegistryUse *use;
        StaticRegistryKeyState state;
        assert(static_registry_use(&registry, path, &use, &state));
        const StaticOccurrence *site = caller_module->first_occurrence;
        while (site && (!site->statement || site->statement->kind != AST_DECLARATION)) { site = site->next; }
        assert(site);
        const StaticTerm *original;
        assert(static_term_inference(&terms, caller_environment, formal, site->statement, NULL, &original));
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        const StaticTerm *output = NULL;
        state = STATIC_REGISTRY_KEY_PENDING;
        bool result = static_registry_scope_variable(&registry, use, original, &output, &state);
        if (!failure) { assert(result && output); total = allocation_count - before; assert(total); }
        else {
            assert(!result && output == NULL && state == STATIC_REGISTRY_KEY_PENDING);
            assert(registry.status == STATIC_REGISTRY_OUT_OF_MEMORY && diagnostic_valid(&registry.error));
        }
        failure_at = 0;
        destroy_registry();
    }
}

/** Exact membership and owner equality reject copied handles, nominal values,
 * foreign scopes and absent receivers before preparing any equation.
 */
static void it_refuses_nonlocal_variables_and_unowned_use_handles(void) {
    for (size_t partition = 0; partition < 6; partition++) {
        setup_registry("@<\nvalue\n...type\n@!\nanchor&\n..integer\n", "@<\nvalue\n...type\n@!\n");
        const StaticCreationPath *path;
        assert(static_creation_root(&guards, caller_environment, &path));
        StaticRegistryUse *use;
        StaticRegistryKeyState state;
        assert(static_registry_use(&registry, path, &use, &state));
        const StaticTerm *local, *foreign;
        const StaticTermEnvironment *target_environment;
        assert(static_term_environment(&terms, target_module, NULL, &target_environment));
        assert(static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &local));
        assert(static_term_symbol(&terms, target_environment, application_formal(target_module, 0), &foreign));
        StaticRegistryUse copied = *use;
        StaticTerm copied_term = *local;
        const StaticTerm *input = partition == 1 ? integer_type : partition == 2 ? foreign :
            partition == 3 ? &copied_term : local;
        const StaticTerm *output = NULL;
        size_t before = allocation_count;
        assert(!static_registry_scope_variable(&registry, partition == 0 ? &copied : use,
            input, partition == 4 ? NULL : &output, partition == 5 ? NULL : &state));
        assert(output == NULL && registry.status == STATIC_REGISTRY_INVALID && allocation_count == before);
        destroy_registry();
    }
}

/** Compiler-only producer formals resolve from their full input maps, while
 * derived graph names remain canonical scoped variables rather than new inputs.
 */
static void it_canonicalizes_producer_formals_and_derived_graph_names(void) {
    setup_registry("@...\n...out\n...type\n...anchor\n..integer\n", "@!\n");
    const StaticSymbol *formal = application_formal(caller_module, 0);
    const StaticTerm *parameter;
    assert(static_term_symbol(&terms, caller_environment, formal, &parameter));
    assert(static_constraint_equate(&solver, parameter, integer_type, formal->site));
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, caller_environment, &path));
    StaticRegistryUse *use;
    StaticRegistryKeyState state;
    assert(static_registry_use(&registry, path, &use, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
    const StaticTerm *canonical;
    assert(static_registry_scope_variable(&registry, use, parameter, &canonical, &state));
    assert(canonical == integer_type && use->candidate->key->producer_applied);
    const StaticSymbol *derived = static_symbol_find(caller_module, static_name_site(project.entry, "...out"));
    assert(derived && derived != formal);
    const StaticTerm *original, *direct;
    assert(static_term_symbol(&terms, caller_environment, derived, &original));
    assert(static_registry_scope_variable(&registry, use, original, &canonical, &state));
    assert(static_term_symbol(&terms, use->candidate->key, derived, &direct));
    assert(canonical == direct && canonical != original);
    assert(use->candidate->key->owner_type == NULL);
    destroy_registry();
}

/** Exercise sharing, exact refusal and every measured preparation failure. */
static void run(void) {
    it_canonicalizes_producer_formals_and_derived_graph_names();
    it_canonicalizes_equal_scopes_and_preserves_local_occurrences();
    it_releases_every_partial_scope_preparation_on_allocation_refusal();
    it_refuses_nonlocal_variables_and_unowned_use_handles();
}
/** Return only after counted teardown has checked every fixture lifetime. */
int main(void) { run(); return 0; }
