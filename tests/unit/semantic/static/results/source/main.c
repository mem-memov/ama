/** @file
 * @brief Verify original receiver identity, owner substitution and all source preparation refusals.
 */
#include "../common.c"

/** Source preparation binds values by exact identities but generates no output
 * equations, inferred producer inputs or new enclosing parameters.
 */
static void it_binds_original_results_and_first_alias_receivers_without_conflating_values(void) {
    setup_results();
    const StaticSymbol *alias_symbol = result_named(caller_module, ".alias");
    const StaticResultDefinition *alias, *first, *partial, *again;
    assert(static_result_definition(&results, caller_environment, alias_symbol, &alias));
    assert(alias_symbol->role == STATIC_ALIAS && alias->receiver == alias_symbol->receiver);
    assert(alias->source->expression == result_projection(0) && alias->source->expression != result_projection(1));
    assert(alias->source->producer == target_module && alias->source->head == NULL && alias->source->value == NULL);
    assert(alias->value->as.variable.role == STATIC_VARIABLE_DERIVED && alias->value->as.variable.symbol == alias_symbol);
    assert(target_constructor == NULL && caller_constructor == NULL);
    const StaticResultSource *read;
    assert(static_result_source(&results, caller_environment, result_pair(".same"), &read));
    assert(read->head == alias->value && read->value != alias->value);
    assert(static_result_definition(&results, caller_environment, result_named(caller_module, ".first"), &first));
    assert(static_result_definition(&results, caller_environment, result_named(caller_module, ".partial"), &partial));
    assert(first->source->head == application_symbol(".type") && first->receiver == NULL);
    assert(partial->source->head == application_symbol(".family") && partial->receiver == NULL);
    assert(first->value != first->source->value && partial->value != partial->source->head);
    size_t before = allocation_count;
    assert(static_result_definition(&results, caller_environment, alias_symbol, &again));
    assert(again == alias && allocation_count == before && results.definition_count == 3);
    assert(caller_module->parameter_count == 2 && solver.equation_count == 0 && guards.path_count == 0);
    destroy_results();
}

/** Equal syntax in two owner environments keeps independent expression and
 * derived definition identities; applied formals read their exact fixed captures.
 */
static void it_preserves_applied_type_and_template_captures_in_distinct_scopes(void) {
    setup_results();
    const StaticTermEnvironment *applied = result_applied_environment();
    const StaticSymbol *first_symbol = result_named(caller_module, ".first");
    const StaticSymbol *partial_symbol = result_named(caller_module, ".partial");
    const StaticResultDefinition *generic, *first, *partial;
    assert(static_result_definition(&results, caller_environment, first_symbol, &generic));
    assert(static_result_definition(&results, applied, first_symbol, &first));
    assert(static_result_definition(&results, applied, partial_symbol, &partial));
    assert(generic != first && generic->source->expression == first->source->expression);
    assert(generic->value != first->value && generic->source->value != first->source->value);
    assert(first->source->head == integer_type && partial->source->head == result_family_constructor);
    assert(first->value->as.variable.environment == applied && generic->value->as.variable.environment == caller_environment);
    assert(caller_module->parameter_count == 2 && target_module->parameter_count == 1);
    assert(solver.equation_count == 0 && terms.environment_count == 2);
    destroy_results();
}

/** Untrusted syntax/environment/symbol copies cannot acquire original identity.
 * Explicit result counts fail before preparing or publishing a new source.
 */
static void it_refuses_foreign_sources_definitions_receivers_and_count_overflow(void) {
    for (size_t mode = 0; mode < 9; mode++) {
        setup_results();
        AstTypeExpression copied_expression = *result_projection(0);
        StaticTermEnvironment copied_environment = *caller_environment;
        StaticSymbol copied_symbol = *result_named(caller_module, ".first");
        const StaticResultSource *source = NULL;
        const StaticResultDefinition *definition = NULL;
        const StaticTermEnvironment *foreign;
        assert(static_term_environment(&terms, target_module, NULL, &foreign));
        if (mode == 3) { results.source_count = SIZE_MAX; }
        if (mode == 8) { results.definition_count = SIZE_MAX; }
        size_t before = allocation_count;
        bool success;
        if (mode < 5) {
            success = static_result_source(&results, mode == 1 ? &copied_environment : caller_environment,
                mode == 0 ? &copied_expression : mode == 2 ? result_named(target_module, ".out")->declaration->as.static_result.expression : result_projection(0),
                mode == 4 ? NULL : &source);
        } else {
            success = static_result_definition(&results, mode == 7 ? foreign : caller_environment,
                mode == 5 ? &copied_symbol : mode == 6 ? application_formal(caller_module, 0) : result_named(caller_module, ".first"), &definition);
        }
        assert(!success && source == NULL && definition == NULL && allocation_count == before);
        assert(results.status == ((mode == 3 || mode == 8) ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
        assert(!static_result_source(&results, caller_environment, result_projection(0), &source));
        assert(allocation_count == before);
        destroy_results();
    }
}

/** Refuse every actual allocation in path/formal/alias/applied source binding.
 * A valid source record may already exist before a later definition allocation
 * fails; no definition output or source inventory is falsely published/mutated.
 */
static void it_unwinds_every_source_and_definition_preparation_allocation_refusal(void) {
    for (size_t mode = 0; mode < 4; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_results();
            const StaticTermEnvironment *environment = mode == 3 ? result_applied_environment() : caller_environment;
            const StaticResultSource *source = NULL;
            const StaticResultDefinition *definition = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = mode == 0 ? static_result_source(&results, environment, result_projection(0), &source) :
                static_result_definition(&results, environment,
                    result_named(caller_module, mode == 1 ? ".first" : mode == 2 ? ".alias" : ".partial"), &definition);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && source == NULL && definition == NULL && results.status == STATIC_RESULT_OUT_OF_MEMORY);
                assert(results.definition_count == 0);
                if (mode == 0) { assert(results.source_count == 0); }
            }
            failure_at = 0;
            assert(inventory.valid && caller_module->parameter_count == 2 && solver.equation_count == 0);
            destroy_results();
        }
    }
}

/** Execute exact source/alias identity, owner substitution and refusal partitions. */
static void run(void) {
    it_binds_original_results_and_first_alias_receivers_without_conflating_values();
    it_preserves_applied_type_and_template_captures_in_distinct_scopes();
    it_refuses_foreign_sources_definitions_receivers_and_count_overflow();
    it_unwinds_every_source_and_definition_preparation_allocation_refusal();
}
/** Return success after zero-live-allocation teardown. */
int main(void) { run(); return 0; }
