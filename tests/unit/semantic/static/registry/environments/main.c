/** @file
 * @brief Verify role-independent canonical scope equations, late variables and exact refusal ownership.
 */
#include "../common.c"

/** Borrow original source symbols and four distinct current static variable roles. */
typedef struct {
    StaticRegistryUse *use;
    const StaticTerm *variables[4];
    const StaticSymbol *alias;
    const StaticSymbol *foreign;
    const AstTypeExpression *pair;
    const AstTypeExpression *projection;
} EnvironmentFixture;

/** Register actual source identities before the owner key is known. */
static EnvironmentFixture setup_environment(void) {
    setup_registry("@...\n...forward\n...type\n.other\n ...out > ...alias\n...anchor\n..integer\n",
        "@...\n...out\n...item\n");
    EnvironmentFixture fixture = {0};
    fixture.alias = static_symbol_find(caller_module, static_name_site(project.entry, "...alias"));
    const StaticSymbol *forward = static_symbol_find(caller_module, static_name_site(project.entry, "...forward"));
    fixture.pair = forward->declaration->as.static_result.expression;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->statement->kind == AST_STATIC_PROJECTION &&
            site->statement->as.static_projection == site->expression) { fixture.projection = site->expression; }
    }
    assert(fixture.alias && fixture.projection);
    fixture.foreign = application_formal(target_module, 0);
    assert(static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &fixture.variables[0]));
    assert(static_term_symbol(&terms, caller_environment, fixture.alias, &fixture.variables[1]));
    assert(static_term_expression_result(&terms, caller_environment, fixture.pair, &fixture.variables[2]));
    assert(static_term_inference(&terms, caller_environment, fixture.foreign, NULL, fixture.projection, &fixture.variables[3]));
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, caller_environment, &path));
    StaticRegistryKeyState state;
    assert(static_registry_use(&registry, path, &fixture.use, &state) && state == STATIC_REGISTRY_KEY_PENDING);
    return fixture;
}

/** The independent public term APIs must retrieve the counterparts already
 * created by the environment scan, retaining original declaration/formal keys.
 */
static void canonical_variables(EnvironmentFixture fixture, const StaticTerm **values) {
    const StaticTermEnvironment *key = fixture.use->candidate->key;
    assert(static_term_symbol(&terms, key, application_formal(caller_module, 0), &values[0]));
    assert(static_term_symbol(&terms, key, fixture.alias, &values[1]));
    assert(static_term_expression_result(&terms, key, fixture.pair, &values[2]));
    assert(static_term_inference(&terms, key, fixture.foreign, NULL, fixture.projection, &values[3]));
}

/** Unknown owners add no equations. Once known, all existing roles and later
 * source variables share canonical values, without conflating source operations
 * or creating an unbounded scan of freshly appended counterparts.
 */
static void it_canonicalizes_all_existing_and_late_variables_only_after_owner_keys_close(void) {
    EnvironmentFixture fixture = setup_environment();
    StaticRegistryKeyState state;
    assert(static_registry_scope_environment(&registry, fixture.use, &state) && state == STATIC_REGISTRY_KEY_PENDING);
    assert(solver.equation_count == 0 && fixture.use->candidate == NULL);
    assert(static_constraint_equate(&solver, fixture.variables[0], integer_type, fixture.variables[0]->site));
    assert(static_registry_scope_environment(&registry, fixture.use, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
    const StaticTerm *canonical[4];
    canonical_variables(fixture, canonical);
    assert(canonical[0] == integer_type);
    for (size_t index = 1; index < 4; index++) {
        assert(canonical[index] != fixture.variables[index]);
        assert(static_constraint_equate(&solver, fixture.variables[index], integer_type, fixture.variables[index]->site));
        const StaticTerm *value;
        StaticConstraintOutcome outcome;
        assert(static_constraint_value(&solver, canonical[index], &value, &outcome));
        assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    }
    const StaticSymbol *forward = static_symbol_find(caller_module, static_name_site(project.entry, "...forward"));
    const StaticTerm *late, *late_canonical;
    assert(static_term_symbol(&terms, caller_environment, forward, &late));
    assert(static_registry_scope_environment(&registry, fixture.use, &state));
    assert(static_term_symbol(&terms, fixture.use->candidate->key, forward, &late_canonical));
    assert(late != late_canonical);
    assert(static_constraint_equate(&solver, late, integer_type, late->site));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, late_canonical, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    const StaticCreationPath *canonical_path;
    assert(static_creation_root(&guards, fixture.use->candidate->key, &canonical_path));
    StaticRegistryUse *canonical_use;
    assert(static_registry_use(&registry, canonical_path, &canonical_use, &state));
    size_t equations = solver.equation_count, count = terms.term_count;
    assert(static_registry_scope_environment(&registry, canonical_use, &state));
    assert(static_registry_scope_environment(&registry, fixture.use, &state));
    assert(solver.equation_count == equations && terms.term_count == count);
    assert(registry.candidate_count == 1 && fixture.use->candidate->state == STATIC_CANDIDATE_NEW);
    destroy_registry();
}

/** Every new variable and equation remains owned when a batch stops partway;
 * the public key-state receiver is changed only on complete success.
 */
static void it_releases_partial_scope_equations_after_every_measured_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        EnvironmentFixture fixture = setup_environment();
        assert(static_constraint_equate(&solver, fixture.variables[0], integer_type, fixture.variables[0]->site));
        StaticRegistryKeyState state;
        assert(static_registry_refresh_keys(&registry, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        state = STATIC_REGISTRY_KEY_PENDING;
        bool result = static_registry_scope_environment(&registry, fixture.use, &state);
        if (!failure) { assert(result && state == STATIC_REGISTRY_KEY_CLOSED); total = allocation_count - before; assert(total); }
        else {
            assert(!result && state == STATIC_REGISTRY_KEY_PENDING && registry.status == STATIC_REGISTRY_OUT_OF_MEMORY);
            assert(fixture.use->candidate->state == STATIC_CANDIDATE_FAILED);
        }
        failure_at = 0;
        destroy_registry();
    }
}

/** Exact handle membership and receiver validity precede variable enumeration. */
static void it_refuses_copied_use_handles_and_missing_state_receivers(void) {
    for (unsigned partition = 0; partition < 2; partition++) {
        EnvironmentFixture fixture = setup_environment();
        StaticRegistryUse copied = *fixture.use;
        StaticRegistryKeyState state = STATIC_REGISTRY_KEY_PENDING;
        size_t before = allocation_count;
        assert(!static_registry_scope_environment(&registry, partition == 0 ? &copied : fixture.use,
            partition == 1 ? NULL : &state));
        assert(state == STATIC_REGISTRY_KEY_PENDING && registry.status == STATIC_REGISTRY_INVALID);
        assert(allocation_count == before && solver.equation_count == 0);
        destroy_registry();
    }
}

/** Run finite scope identity, late discovery and exact failure partitions. */
static void run(void) {
    it_canonicalizes_all_existing_and_late_variables_only_after_owner_keys_close();
    it_releases_partial_scope_equations_after_every_measured_allocation_refusal();
    it_refuses_copied_use_handles_and_missing_state_receivers();
}
/** Return after counted unconditional teardown. */
int main(void) { run(); return 0; }
