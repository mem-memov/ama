/** @file
 * @brief Verify symbolic field identities, exact owner congruence and metadata/context separation.
 */
#include "common.c"

/** Every original field gets a required Type term while native bodies/defaults
 * and source application/definition evaluation remain completely untouched.
 */
static void it_prepares_explicit_inferred_and_local_type_terms_without_evaluating_bodies(void) {
    setup_typed();
    const StaticResultContext *actual = typed_actual();
    size_t paths = guards.path_count;
    const StaticTypedScope *scope, *again;
    assert(static_connection_scope(&connections, caller_environment, &scope));
    assert(scope->field_count == 8 && scope->environment == caller_environment && connections.context_count == 0);
    size_t sources_count = 0, holes = 0;
    for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
        StaticCategory category;
        const StaticSignature *signature;
        static_constraint_description(&solver, field->type, &category, &signature);
        assert(category == STATIC_CATEGORY_TYPE && signature == NULL);
        bool required = false;
        for (const StaticConstraintRequirement *item = solver.first_requirement; item; item = item->next) { required |= item->term == field->type; }
        assert(required);
        if (field->type_source) { sources_count++; assert(field->type_source->expression == field->definition->type_expression); }
        else { holes++; assert(field->type->as.variable.role == STATIC_VARIABLE_RUNTIME && field->type->as.variable.symbol == NULL); }
    }
    assert(sources_count == 5 && holes == 3 && guards.path_count == paths && results.use_count == 0);
    assert(typed_field(scope, "fresh")->type != typed_field(scope, "borrowed")->type);
    assert(static_connection_context(&connections, actual, &again) && again == scope && connections.context_count == 1);
    size_t before = allocation_count;
    failure_at = before + 1;
    assert(static_connection_scope(&connections, caller_environment, &again) && again == scope);
    assert(static_connection_context(&connections, actual, &again) && again == scope && allocation_count == before);
    failure_at = 0;
    destroy_connections();
}

/** A supplied exact nominal environment can expose field metadata, but cannot
 * manufacture an actual root/context or authorize body expansion/readiness.
 */
static void it_keeps_metadata_only_nominal_scopes_separate_from_actual_provenance(void) {
    setup_typed();
    const StaticTermEnvironment *environment = typed_environment(integer_type);
    const StaticTypedScope *scope;
    assert(static_connection_scope(&connections, environment, &scope));
    assert(scope->owner == environment->owner_type && scope->field_count == 8);
    assert(connections.context_count == 0 && results.context_count == 0 && guards.path_count == 0 && results.use_count == 0);
    assert(scope->runtime->source == caller_module);
    destroy_connections();
}

/** Existing owner-expression identity proves field congruence even while the
 * owner key remains unknown. No free owner argument is guessed or closed.
 */
static void it_joins_corresponding_symbolic_fields_without_closing_unknown_owner_keys(void) {
    setup_typed();
    const StaticResultContext *actual = typed_actual();
    const StaticTypedScope *root, *metadata;
    assert(static_connection_context(&connections, actual, &root));
    const StaticTerm *slot = application_symbol("...slot");
    assert(static_connection_scope(&connections, typed_environment(slot), &metadata));
    assert(root != metadata && root->owner == metadata->owner && root->runtime == metadata->runtime);
    const StaticTypedField *left = typed_field(root, "fresh"), *right = typed_field(metadata, "fresh");
    assert(left->type != right->type);
    bool changed = false;
    assert(static_connection_merge_fields(&connections, &changed) && changed);
    bool equal = false;
    assert(static_constraint_equivalent(&solver, left->type, right->type, &equal) && equal);
    assert(static_constraint_equate(&solver, left->type, integer_type, left->definition->site));
    const StaticTerm *value = NULL;
    StaticConstraintOutcome closed;
    assert(static_constraint_value(&solver, right->type, &value, &closed) && closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    value = NULL;
    assert(static_constraint_value(&solver, root->owner, &value, &closed) && closed == STATIC_CONSTRAINT_PENDING && value == NULL);
    assert(static_connection_merge_fields(&connections, &changed) && !changed);
    assert(connections.context_count == 1 && guards.path_count == 1 && results.use_count == 0);
    destroy_connections();
}

/** Different unknown bindings do not agree merely because their origins match.
 * A later genuine owner anchor exposes a conflicting same-field Type equation.
 */
static void it_defers_unproved_owner_equality_and_rejects_later_same_owner_field_conflicts(void) {
    setup_typed();
    const StaticTypedScope *root, *metadata;
    assert(static_connection_context(&connections, typed_actual(), &root));
    assert(static_connection_scope(&connections, typed_environment(integer_type), &metadata));
    const StaticTypedField *left = typed_field(root, "fresh"), *right = typed_field(metadata, "fresh");
    const StaticModule *text = static_module_find(&inventory, project_find(&project, "..text"));
    const StaticTerm *text_type;
    assert(text && static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &text_type));
    assert(static_constraint_equate(&solver, left->type, integer_type, left->definition->site));
    assert(static_constraint_equate(&solver, right->type, text_type, right->definition->site));
    bool changed = true;
    assert(static_connection_merge_fields(&connections, &changed) && !changed);
    const StaticTerm *slot = application_symbol("...slot");
    assert(static_constraint_equate(&solver, slot, integer_type, slot->site));
    assert(static_connection_merge_fields(&connections, &changed) && changed);
    StaticConstraintOutcome outcome;
    assert(!static_constraint_solve(&solver, &outcome) && solver.status == STATIC_CONSTRAINT_CONFLICT);
    assert(!static_connection_dependencies(&connections) && connections.status == STATIC_CONNECTION_CONFLICT);
    assert(diagnostic_valid(&connections.error));
    destroy_connections();
}

/** Term identities retain source/environment owners, never shorter-lived typed
 * field records. Solver use remains valid after the connection arena is destroyed.
 */
static void it_keeps_runtime_type_holes_valid_after_typed_association_owner_teardown(void) {
    setup_typed();
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, typed_actual(), &scope));
    const StaticTypedField *field = typed_field(scope, "fresh");
    const StaticTerm *type = field->type;
    SourceSpan site = field->definition->site;
    static_connection_destroy(&connections);
    assert(static_constraint_equate(&solver, type, integer_type, site));
    const StaticTerm *value;
    StaticConstraintOutcome closed;
    assert(static_constraint_value(&solver, type, &value, &closed) && closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    destroy_constants();
}

/** Run source/term identity, proof-only joins, separate provenance and ownership. */
static void run(void) {
    it_prepares_explicit_inferred_and_local_type_terms_without_evaluating_bodies();
    it_keeps_metadata_only_nominal_scopes_separate_from_actual_provenance();
    it_joins_corresponding_symbolic_fields_without_closing_unknown_owner_keys();
    it_defers_unproved_owner_equality_and_rejects_later_same_owner_field_conflicts();
    it_keeps_runtime_type_holes_valid_after_typed_association_owner_teardown();
}
/** Return only after all assertions and counted teardown. */
int main(void) { run(); return 0; }
