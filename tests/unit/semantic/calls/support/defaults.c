/** @file
 * @brief Inspect exact omission metadata, independent interface positions and caller-owned source/receiver views.
 */
/** Placeholders retain defaults/skips without source or receiver allocation and keep independent input/output positions. */
static void it_preserves_independent_positions_defaults_skips_and_literal_bytes(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\nfirst\n/test/fixture/v1.0.0/src/value\n |1\nsecond\n/test/fixture/v1.0.0/src/resource\n@>\nresult\n/test/fixture/v1.0.0/src/value\n |2\nresource\n/test/fixture/v1.0.0/src/resource\n@!\n");
    SemanticStatus status = resolve_text("@!\nworker\n/test/fixture/v1.0.0/src/worker\nreceived\n/test/fixture/v1.0.0/src/value\nworker !\n < |bytes # data  \n <\n > received\n >\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *call = semantic_entry()->last_statement;
    SemanticBinding *input = call->bindings;
    assert(input->position == 0 && input->transfer == TRANSFER_LITERAL);
    assert(layout_text_is(input->source->syntax->payload, "bytes # data  "));
    SemanticBinding *omitted = input->next;
    assert(omitted->position == 1 && omitted->transfer == TRANSFER_DEFAULT);
    assert(omitted->source == NULL && omitted->receiver == NULL);
    SemanticBinding *output = omitted->next;
    assert(output->position == 0 && output->transfer == TRANSFER_CLONE);
    assert(output->source->declaration == output->field && output->source->syntax == NULL);
    assert(output->next->position == 1 && output->next->transfer == TRANSFER_DEFAULT);
    assert(output->next->source == NULL && output->next->receiver == NULL && output->next->next == NULL);
    destroy_semantic();
    expect_resolved("@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\nworker %\n >\n >\n", true);
}
/** Same-spelled caller values and interface labels retain independent owners and exact text-order bindings. */
static void it_resolves_sources_and_receivers_in_the_caller_not_the_field_owner(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\nother\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\nsecond result\n/test/fixture/v1.0.0/src/value\n@!\n");
    SemanticStatus status = resolve_text("@!\ninput\n/test/fixture/v1.0.0/src/value\noutput\n/test/fixture/v1.0.0/src/value\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n other < input\n input < output\n output > input\n output > output\n");
    /* Repeating a callee label is forbidden even when caller receivers differ. */
    assert(status == SEMANTIC_INVALID);
    destroy_semantic();
    status = resolve_text("@!\ninput\n/test/fixture/v1.0.0/src/value\noutput\n/test/fixture/v1.0.0/src/value\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n other < input\n input < output\n output > input\n second result > input\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *call = semantic_entry()->last_statement;
    assert(call->bindings->position == 1 && call->bindings->next->position == 0);
    for (SemanticBinding *binding = call->bindings; binding != NULL; binding = binding->next) {
        assert(binding->field->owner == call->namespace_type && binding->field->owner != semantic_entry());
        if (binding->receiver != NULL) { assert(binding->receiver->symbol->owner == semantic_entry()); }
        else { assert(binding->source->access->symbol->owner == semantic_entry()); }
    }
    destroy_semantic();
}
