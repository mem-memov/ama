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
    assert(static_constraint_equate(&solver, application_symbol("...type"), integer_type, result_named(caller_module, "...type")->site));
    StaticResultDefinitionUse *definition = checked_bare(generic, "...first");
    bool cached = true;
    assert(static_result_cache_store(&results, definition, &cached) && !cached && results.cache_count == 0);
    assert(static_constraint_equate(&solver, application_symbol("...family"), target_constructor,
        result_named(caller_module, "...family")->site));
    assert(static_result_cache_store(&results, definition, &cached) && cached && results.cache_count == 1);
    assert(results.first_cache->value == integer_type);
    const StaticResultContext *applied = cache_context(result_applied_environment());
    StaticResultDefinitionUse *other;
    assert(static_result_definition_use(&results, applied, result_named(caller_module, "...first"), NULL, (SourceSpan){0}, &other));
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
    StaticResultDefinitionUse *partial = setup_checked("...partial");
    bool cached;
    assert(static_result_cache_store(&results, partial, &cached) && cached);
    assert(results.first_cache->value == target_constructor && target_constructor->as.nominal.residual_count == 1);
    StaticResultDefinitionUse *first = checked_bare(partial->context, "...first");
    assert(static_result_cache_store(&results, first, &cached) && cached && results.cache_count == 2);
    assert(first->definition->symbol != partial->definition->symbol);
    assert(static_result_cache_read(&results, partial, &cached) && cached && partial->cache_hit);
    size_t count = allocation_count;
    assert(static_result_cache_read(&results, partial, &cached) && cached && allocation_count == count);
    destroy_results();
}

/** The selected constant integer says nothing about the producer's unused
 * Type input. This source-backed fixture prepares the exact static equations;
 * no generic evaluator, module body or native hook is executed here.
 */
static void it_keeps_constant_alias_values_uncached_until_every_producer_input_closes(void) {
    setup_guards("@>\n...first\n..integer\n@!\nanchor&\n..integer\nfirst&\n.other\n ...out > ...alias\n",
        "@<\n...unused\n@>\n...out\n..integer\n@!\n");
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = cache_context(caller_environment);
    StaticResultDefinitionUse *alias;
    assert(static_result_definition_use(&results, context, result_named(caller_module, "...alias"), NULL, (SourceSpan){0}, &alias));
    StaticResultUse *rhs;
    assert(static_result_use(&results, context, result_field("first"), alias->path, &rhs));
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, rhs->source->expression, &requests));
    assert(static_term_application(&terms, &binding_plan, requests, caller_environment, rhs->source->head,
        NULL, 0, &rhs->prepared));
    assert(static_application_source(&applications, rhs->prepared, &rhs->application));
    const StaticCreationPath *producer;
    StaticCreationOutcome created;
    assert(static_creation_enter(&guards, context->creation, rhs->application, &producer, &created));
    assert(created == STATIC_CREATION_ENTERED && static_result_context(&results, producer, &rhs->producer));
    const StaticSymbol *out_symbol = result_named(target_module, "...out");
    assert(rhs->application->context.requests->last->slot->static_symbol == out_symbol);
    StaticResultDefinitionUse *out;
    assert(static_result_definition_use(&results, rhs->producer, out_symbol, alias->path,
        rhs->source->expression->span, &out));
    StaticResultUse *out_rhs;
    assert(static_result_use(&results, rhs->producer, out->definition->source->expression, out->path, &out_rhs));
    assert(out_rhs->source->head == integer_type);
    StaticApplicationContext output_context;
    assert(static_application_context(&applications, producer->owner->environment, out_rhs->source->expression, &output_context));
    output_context.result = out_rhs->source->value;
    assert(static_application_forward(&applications, integer_type, NULL, 0, output_context, &out_rhs->application));
    const StaticCreationPath *native;
    assert(static_creation_enter(&guards, producer, out_rhs->application, &native, &created));
    assert(created == STATIC_CREATION_ENTERED && static_result_context(&results, native, &out_rhs->producer));
    assert(static_constraint_equate(&solver, out->definition->value, out_rhs->source->value, out_symbol->site));
    assert(static_constraint_equate(&solver, alias->definition->value, out->definition->value, alias->definition->receiver->span));
    out_rhs->outcome = STATIC_RESULT_READY;
    out_rhs->closed = true;
    out->rhs = out_rhs;
    out->outcome = STATIC_RESULT_READY;
    alias->rhs = rhs;
    alias->outcome = STATIC_RESULT_READY;
    rhs->outcome = STATIC_RESULT_READY;
    const StaticTerm *value = NULL;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, alias->definition->value, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(static_constraint_value(&solver, rhs->source->value, &value, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
    bool cached = true;
    assert(static_result_cache_store(&results, alias, &cached) && !cached && results.cache_count == 0);
    const StaticTerm *unused = rhs->application->value->as.nominal.first_capture->value;
    assert(unused->as.variable.role == STATIC_VARIABLE_PRODUCER);
    assert(static_constraint_equate(&solver, unused, integer_type, unused->site));
    assert(static_constraint_value(&solver, rhs->source->value, &value, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    rhs->closed = true;
    assert(static_result_cache_store(&results, alias, &cached) && cached && results.cache_count == 1);
    assert(results.first_cache->value == integer_type && caller_module->parameter_count == 0);
    destroy_results();
}

/** Source and definition work must be complete; copied uses, missing outputs,
 * mismatched original RHS pointers and counter overflow cannot publish entries.
 */
static void it_refuses_unchecked_foreign_mismatched_and_overflowing_cache_publication(void) {
    for (size_t mode = 0; mode < 6; mode++) {
        StaticResultDefinitionUse *use = setup_checked("...first");
        StaticResultDefinitionUse copied = *use;
        bool cached = false;
        if (mode == 0) { use->outcome = STATIC_RESULT_PENDING; }
        if (mode == 1) { use->rhs->closed = false; }
        if (mode == 4) {
            StaticResultUse *other;
            assert(static_result_use(&results, use->context, result_field("first"), use->path, &other));
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
            StaticResultDefinitionUse *use = setup_checked("...first");
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
