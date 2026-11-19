/** @file
 * @brief Verify exact named producer maps, lexical omitted-input identity and all preparation failures.
 */
#include "common.c"

/** Bind an exact source statement before measuring request preparation effects. */
static const StaticResultSource *producer_source(size_t ordinal) {
    const StaticResultSource *source;
    assert(static_result_source(&results, caller_environment, result_projection(ordinal), &source));
    assert(source->producer == target_module && source->head == NULL && source->value == NULL);
    return source;
}


/** Full original input maps are independent of supplied-binding order, preserve
 * explicit lexical child values and invent no producer Type or caller input.
 */
static void it_prepares_complete_named_maps_with_stable_occurrence_local_holes(void) {
    setup_producer_requests(request_caller, request_target);
    const StaticResultSource *first = producer_source(0), *second = producer_source(1);
    const StaticResultProducer *prepared, *again, *other;
    assert(static_result_prepare_producer(&results, first, &prepared));
    assert(prepared->source == first && prepared->input_count == 2);
    assert(prepared->requests->interface->origin == target_module && !prepared->requests->pending);
    assert(prepared->requests->input_count == 1 && prepared->requests->output_count == 1);
    assert(prepared->requests->interface->sequences[BINDING_STATIC_OUTPUT].count == 2);
    const StaticSymbol *left = application_formal(target_module, 0), *right = application_formal(target_module, 1);
    assert(layout_text_is(left->name, "...left") && layout_text_is(right->name, "...right"));
    assert(prepared->inputs[0].formal == left && prepared->inputs[1].formal == right);
    const BindingRequest *input = prepared->requests->first;
    assert(input->direction == BINDING_STATIC_INPUT && input->slot->static_symbol == left);
    const StaticResultSource *child;
    assert(static_result_source(&results, caller_environment, input->syntax.static_binding->input, &child));
    assert(child->head == integer_type && prepared->inputs[0].value == child->value);
    const StaticTerm *hole = prepared->inputs[1].value;
    assert(hole->kind == STATIC_TERM_VARIABLE && hole->as.variable.role == STATIC_VARIABLE_PRODUCER);
    assert(hole->as.variable.symbol == right && hole->as.variable.producer == first->expression);
    assert(hole->as.variable.environment == caller_environment);
    size_t before = allocation_count;
    assert(static_result_prepare_producer(&results, first, &again) && again == prepared);
    assert(allocation_count == before);
    assert(static_result_prepare_producer(&results, second, &other));
    assert(other != prepared && other->input_count == 2);
    assert(other->inputs[1].value != hole && other->inputs[0].value != prepared->inputs[0].value);
    assert(other->inputs[1].value->as.variable.producer == second->expression);
    assert(caller_module->parameter_count == 0 && guards.path_count == 0 && solver.equation_count == 0);
    assert(target_constructor == NULL && caller_constructor == NULL && results.producer_request_count == 2);
    destroy_results();
}

/** A determined zero-input producer still denotes named outputs rather than an
 * ordinary metadata value; its complete input array is genuinely empty.
 */
static void it_prepares_zero_input_producers_without_an_implicit_type_or_instance(void) {
    setup_producer_requests("@...\n.other\n ...out > ...received\n...anchor\n..integer\n",
        "@...\n...out\n..integer\n");
    const StaticResultSource *source = producer_source(0);
    const StaticResultProducer *prepared;
    assert(static_result_prepare_producer(&results, source, &prepared));
    assert(prepared->inputs == NULL && prepared->input_count == 0 && prepared->requests->input_count == 0);
    assert(prepared->requests->output_count == 1 && source->value == NULL && guards.path_count == 0);
    destroy_results();
}

/** Exact source ownership and kind are checked before any cache or foreign
 * interface access. Dense counters refuse wrap without changing the receiver.
 */
static void it_refuses_foreign_nonproducer_sources_missing_receivers_and_counter_overflow(void) {
    for (unsigned partition = 0; partition < 4; partition++) {
        setup_producer_requests(request_caller, request_target);
        const StaticResultSource *source = producer_source(0), *ordinary;
        assert(static_result_source(&results, caller_environment, result_pair("...anchor"), &ordinary));
        StaticResultSource copied = *source;
        if (partition == 3) { results.producer_request_count = SIZE_MAX; }
        const StaticResultProducer *output = NULL;
        size_t before = allocation_count;
        assert(!static_result_prepare_producer(&results, partition == 0 ? &copied : partition == 1 ? ordinary : source,
            partition == 2 ? NULL : &output));
        assert(output == NULL && allocation_count == before);
        assert(results.status == (partition == 3 ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
        destroy_results();
    }
}

/** Refuse every interface, named-request, child-term, inference-hole, map and
 * cache-record allocation in a fresh owner with counted unconditional teardown.
 */
static void it_releases_every_partial_producer_preparation_after_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_producer_requests(request_caller, request_target);
        const StaticResultSource *source = producer_source(0);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        const StaticResultProducer *output = NULL;
        bool result = static_result_prepare_producer(&results, source, &output);
        if (!failure) { assert(result && output); total = allocation_count - before; assert(total); }
        else {
            assert(!result && output == NULL && results.status == STATIC_RESULT_OUT_OF_MEMORY);
            assert(results.producer_request_count == 0 && results.first_producer_request == NULL);
        }
        failure_at = 0;
        assert(caller_module->parameter_count == 0 && guards.path_count == 0 && solver.equation_count == 0);
        destroy_results();
    }
}

/** Foreign names belong to the actual producer interface; equal repeated
 * bindings are errors rather than merged input facts or duplicate selections.
 */
static void it_rejects_unknown_and_repeated_named_inputs_and_outputs(void) {
    const char *sources[] = {
        "@...\n.other\n ...missing < ..integer\n ...out > ...received\n...anchor\n..integer\n",
        "@...\n.other\n ...left < ..integer\n ...left < ..integer\n ...out > ...received\n...anchor\n..integer\n",
        "@...\n.other\n ...missing > ...received\n...anchor\n..integer\n",
        "@...\n.other\n ...out > ...received\n ...out > ...other receipt\n...anchor\n..integer\n"
    };
    for (size_t index = 0; index < sizeof(sources) / sizeof(*sources); index++) {
        setup_producer_requests(sources[index], request_target);
        const StaticResultSource *source = producer_source(0);
        const StaticResultProducer *output = NULL;
        assert(!static_result_prepare_producer(&results, source, &output));
        assert(output == NULL && results.producer_request_count == 0 && results.first_producer_request == NULL);
        assert(results.status == STATIC_RESULT_CONFLICT && binding_plan.status == BINDING_INVALID);
        assert(results.error.code == DIAGNOSTIC_TYPE_MISMATCH && results.error.message == binding_plan.error.message);
        assert(diagnostic_valid(&results.error));
        destroy_results();
    }
}

/** Run source-map, empty-map, exact membership and exhaustive refusal partitions. */
static void run(void) {
    it_rejects_unknown_and_repeated_named_inputs_and_outputs();
    it_prepares_complete_named_maps_with_stable_occurrence_local_holes();
    it_prepares_zero_input_producers_without_an_implicit_type_or_instance();
    it_refuses_foreign_nonproducer_sources_missing_receivers_and_counter_overflow();
    it_releases_every_partial_producer_preparation_after_allocation_refusal();
}
/** Return only after every counted fixture owner has released its allocations. */
int main(void) { run(); return 0; }
