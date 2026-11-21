/** @file
 * @brief Verify source-order receiver identity, later explicit declarations and Type-independent storage.
 */
#include "../common.c"

/** Later typed anchors reuse the exact earlier result-inferred field. Callee
 * labels/static names never consume the receiver's first module occurrence.
 */
static void it_binds_result_and_assignment_receivers_before_any_type_is_concrete(void) {
    setup_connections("@>\nreturned\n.type\n@!\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n received > received\n"
        "known\n//integer\n |7\nreceived\n known\n", "@>\nreceived\n//integer\n@!\n");
    const StaticRuntimeModule *module, *again;
    assert(static_connection_runtime(&connections, caller_module, &module));
    assert(module->field_count == 4 && module->receiver_count == 2);
    const StaticRuntimeReceiver *first = module->first_receiver, *second = first->next;
    assert(first->introduced && first->field->inferred && !second->introduced && second->field == first->field);
    assert(layout_text_is(first->field->name, "received") && first->field->type_expression == NULL);
    assert(module->first_field->role == STATIC_FIELD_RESULT && module->first_field->type_expression != NULL);
    assert(caller_module->parameter_count == 1 && solver.equation_count == 0 && applications.count == 0);
    size_t before = allocation_count;
    failure_at = before + 1;
    assert(static_connection_runtime(&connections, caller_module, &again));
    assert(again == module && allocation_count == before && connections.runtime_count == 1);
    failure_at = 0;
    destroy_connections();
}

/** Multiword base names are indivisible. Source kind/suffixes are preserved for
 * later transfer equations without manufacturing a concrete nominal Type.
 */
static void it_retains_simple_assignment_reference_list_and_continuation_receiver_forms(void) {
    setup_connections("@!\nknown value\n//integer\nknown values[]\n//integer\nknown refs&[]\n//integer\n"
        "new value\n known value\nnew ref&\n & known value\nnew values[]\n known values[]\nnew refs&[]\n known refs&[]\n"
        "continued\n % known value\n", "@!\n");
    const StaticRuntimeModule *module;
    assert(static_connection_runtime(&connections, caller_module, &module));
    assert(module->field_count == 8 && module->receiver_count == 5 && module->foreach_count == 0);
    const bool reference[] = {false, true, false, true, false};
    const bool list[] = {false, false, true, true, false};
    size_t index = 0;
    for (const StaticRuntimeReceiver *receiver = module->first_receiver; receiver; receiver = receiver->next) {
        assert(index < 5 && receiver->introduced && receiver->field->reference == reference[index] && receiver->field->list == list[index]);
        assert(receiver->field->statement == receiver->statement && receiver->field->receiver == receiver->syntax);
        index++;
    }
    assert(index == 5 && module->encounter_count == 8 && solver.equation_count == 0);
    destroy_connections();
}

/** A hoisted future explicit declaration duplicates a fresh receiver; earlier
 * undefined uses/self-sources and wrong existing suffixes cannot be repaired.
 */
static void it_rejects_future_explicit_duplicates_self_sources_and_unresolved_prior_uses(void) {
    const char *sources[] = {
        "@!\nknown\n//integer\nfresh\n known\nfresh\n//integer\n",
        "@!\nanchor&\n//integer\nfresh\n fresh\n",
        "@!\nknown\n//integer\nfresh !\nfresh\n known\n",
        "@!\nknown\n//integer\nvalues[]\n//integer\nknown[]\n values[]\n",
        "@=\nfixed\n//integer\n@!\nanchor&\n//integer\nfixed\n anchor&\n"
    };
    for (size_t index = 0; index < sizeof(sources) / sizeof(*sources); index++) {
        setup_connections(sources[index], "@!\n");
        const StaticRuntimeModule *module = NULL;
        assert(!static_connection_runtime(&connections, caller_module, &module));
        assert(module == NULL && connections.status == STATIC_CONNECTION_CONFLICT && connections.runtime_count == 0);
        assert(diagnostic_valid(&connections.error));
        if (index == 0) {
            assert(connections.error.related_count == 1);
            assert(connections.error.primary.span.start > connections.error.related[0].location.span.start);
        }
        const char *message = connections.error.message;
        assert(!static_connection_runtime(&connections, caller_module, &module) && connections.error.message == message);
        destroy_connections();
    }
}

/** Every runtime role shares one base-name namespace; static formals remain
 * separate and their multiword components are never individual declarations.
 */
static void it_rejects_duplicate_runtime_roles_without_reclassifying_static_names(void) {
    setup_connections("@=\nsame\n//integer\n@<\nsame&\n.some type\n@!\n", "@!\n");
    const StaticRuntimeModule *module = NULL;
    assert(!static_connection_runtime(&connections, caller_module, &module));
    assert(module == NULL && connections.status == STATIC_CONNECTION_CONFLICT && connections.error.related_count == 1);
    assert(caller_module->parameter_count == 1 && diagnostic_valid(&connections.error));
    destroy_connections();
}

/** Execute source identity, storage and strict classification refusal partitions. */
static void run(void) {
    it_binds_result_and_assignment_receivers_before_any_type_is_concrete();
    it_retains_simple_assignment_reference_list_and_continuation_receiver_forms();
    it_rejects_future_explicit_duplicates_self_sources_and_unresolved_prior_uses();
    it_rejects_duplicate_runtime_roles_without_reclassifying_static_names();
}
/** Return only after every assertion and counted teardown. */
int main(void) { run(); return 0; }
