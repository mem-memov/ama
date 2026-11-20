/** @file
 * @brief Verify local closure, partial-family results and exact cache ownership/failure laws.
 */
#include "common.c"
#include "semantic/static/bindings/normalize.h"

/** An unused unknown family parameter keeps the producer key open even when
 * this result is integer. After independent family evidence, another equivalent
 * applied scope reuses the value while retaining its own derived variable.
 */
static void it_caches_only_complete_owner_keys_and_preserves_incoming_scoped_variables(void) {
    setup_results();
    const StaticResultContext *generic = cache_context(caller_environment);
    assert(static_constraint_equate(&solver, application_symbol(".type"), integer_type, result_named(caller_module, ".type")->site));
    StaticResultDefinitionUse *definition = checked_bare(generic, ".first");
    bool cached = true;
    assert(static_result_cache_store(&results, definition, &cached) && !cached && results.cache_count == 0);
    assert(static_constraint_equate(&solver, application_symbol(".family"), result_family_constructor,
        result_named(caller_module, ".family")->site));
    assert(static_result_cache_store(&results, definition, &cached) && cached && results.cache_count == 1);
    assert(results.first_cache->value == integer_type);
    const StaticResultContext *applied = cache_context(result_applied_environment());
    StaticResultDefinitionUse *other;
    assert(static_result_definition_use(&results, applied, result_named(caller_module, ".first"), NULL, (SourceSpan){0}, &other));
    assert(other->definition->value != definition->definition->value && other->rhs == NULL);
    assert(static_result_cache_read(&results, other, &cached) && cached && other->cache_hit && other->outcome == STATIC_RESULT_READY);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, other->definition->value, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type && results.cache_count == 1);
    destroy_results();
}

/** A returned constructor's own local residual is not an unresolved producer
 * input. Its original origin/captures/signature survive complete value caching.
 */
static void it_preserves_closed_partial_template_values_and_distinct_result_symbols(void) {
    StaticResultDefinitionUse *partial = setup_checked(".partial");
    bool cached;
    assert(static_result_cache_store(&results, partial, &cached) && cached);
    assert(results.first_cache->value == result_family_constructor && result_family_constructor->as.nominal.residual_count == 1);
    StaticResultDefinitionUse *first = checked_bare(partial->context, ".first");
    assert(static_result_cache_store(&results, first, &cached) && cached && results.cache_count == 2);
    assert(first->definition->symbol != partial->definition->symbol);
    assert(static_result_cache_read(&results, partial, &cached) && cached && partial->cache_hit);
    size_t count = allocation_count;
    assert(static_result_cache_read(&results, partial, &cached) && cached && allocation_count == count);
    destroy_results();
}

/** A known selected value does not close an unused producer input. The actual
 * evaluator retains Pending until independent metadata supplies that input.
 */
static void it_keeps_constant_alias_values_uncached_until_every_producer_input_closes(void) {
    setup_guards("@...\n/test/fixture/v1.0.0/src/other\n .out > .alias\n",
        "@...\n.out\n//integer\n.other\n.unused\n");
    initialize_results();
    const StaticResultContext *context = cache_context(caller_environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, context, result_named(caller_module, ".alias"), &outcome));
    assert(outcome == STATIC_RESULT_PENDING);
    StaticResultDefinitionUse *alias;
    assert(static_result_definition_use(&results, context, result_named(caller_module, ".alias"), NULL, (SourceSpan){0}, &alias));
    StaticResultUse *rhs = alias->rhs;
    assert(rhs->producer_request && rhs->source->value == NULL && !rhs->closed);
    size_t before = results.cache_count;
    const StaticTerm *unused = rhs->producer_request->inputs[0].value;
    assert(static_constraint_equate(&solver, unused, integer_type, unused->site));
    assert(static_result_evaluate_definition(&results, context, result_named(caller_module, ".alias"), &outcome));
    assert(outcome == STATIC_RESULT_READY && results.cache_count > before);
    const StaticTerm *term, *value;
    StaticConstraintOutcome solved;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, ".alias"), &term));
    assert(static_constraint_value(&solver, term, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == integer_type && caller_module->parameter_count == 0);
    destroy_results();
}

/** Source and definition work must be complete; copied uses, missing outputs,
 * mismatched original RHS pointers and counter overflow cannot publish entries.
 */
static void it_refuses_unchecked_foreign_mismatched_and_overflowing_cache_publication(void) {
    for (size_t mode = 0; mode < 6; mode++) {
        StaticResultDefinitionUse *use = setup_checked(".first");
        StaticResultDefinitionUse copied = *use;
        bool cached = false;
        if (mode == 0) { use->outcome = STATIC_RESULT_PENDING; }
        if (mode == 1) { use->rhs->closed = false; }
        if (mode == 4) {
            StaticResultUse *other;
            assert(static_result_use(&results, use->context, result_pair(".same"), use->path, &other));
            other->outcome = STATIC_RESULT_READY;
            other->closed = true;
            use->rhs = other;
        }
        if (mode == 5) { results.cache_count = SIZE_MAX; }
        bool success = static_result_cache_store(&results, mode == 2 ? &copied : use, mode == 3 ? NULL : &cached);
        if (mode < 2) { assert(success && !cached && results.cache_count == 0 && results.status == STATIC_RESULT_OK); }
        else {
            assert(!success && !cached);
            assert(results.status == (mode == 5 ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
        }
        destroy_results();
    }
}

/** Every allocation in complete publication and guarded cache-hit equality is
 * refused once. A failed read preserves output/state; owned prior entries survive.
 */
static void it_unwinds_every_cache_publication_and_reuse_allocation_refusal(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            StaticResultDefinitionUse *use = setup_checked(".first");
            bool cached = false;
            if (mode) { assert(static_result_cache_store(&results, use, &cached) && cached); }
            cached = false;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = mode ? static_result_cache_read(&results, use, &cached) : static_result_cache_store(&results, use, &cached);
            if (!failure) { assert(success && cached); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && !cached && results.status == STATIC_RESULT_OUT_OF_MEMORY);
                assert(results.cache_count == (mode ? 1 : 0) && !use->cache_hit);
            }
            failure_at = 0;
            assert(inventory.valid);
            destroy_results();
        }
    }
}

/** Execute complete-key, partial-result, malformed and all allocation partitions. */
static void run(void) {
    it_caches_only_complete_owner_keys_and_preserves_incoming_scoped_variables();
    it_preserves_closed_partial_template_values_and_distinct_result_symbols();
    it_keeps_constant_alias_values_uncached_until_every_producer_input_closes();
    it_refuses_unchecked_foreign_mismatched_and_overflowing_cache_publication();
    it_unwinds_every_cache_publication_and_reuse_allocation_refusal();
}
/** Return success after complete counted teardown. */
int main(void) { run(); return 0; }
