/** @file
 * @brief Check named mapping, deferred origins, independent nested owners and fixed residuals.
 */

/** Direction-local source order differs from actual metadata order. Both input
 * and output labels retain original identity without any positional mode.
 */
static void it_maps_named_graph_requests_to_independent_actual_slots(void) {
    setup_named_requests("@...\n/test/fixture/v1.0.0/src/other\n .b < .value\n .a < .value\n"
        " .second > .two\n .out > .one\n");
    BindingInterface *view = named_interface(NULL, 0);
    BindingApplication *application = NULL;
    assert(binding_normalize_static(&binding_plan, view, expression, &application));
    assert(application->mode == BINDING_MODE_NAMED && !application->pending);
    assert(application->input_count == 2 && application->output_count == 2);
    const BindingRequest *request = application->first;
    const AstStaticBinding *syntax = expression->first;
    for (size_t index = 0; index < 4; index++) {
        assert(request != NULL && request->syntax.static_binding == syntax);
        assert(request->source_ordinal == index % 2 && request->slot->original_ordinal == 1 - index % 2);
        assert(request->named && !request->omitted);
        request = request->next;
        syntax = syntax->next;
    }
    assert(request == NULL && syntax == NULL);
    destroy_named_requests();
}

/** Learning an actual origin creates a new immutable result. The unresolved
 * record retains its null slots and exact original syntax.
 */
static void it_retains_pending_named_requests_when_resolving_an_independent_origin(void) {
    setup_named_requests("@...\n/test/fixture/v1.0.0/src/other\n .a < .value\n .out > .one\n");
    BindingApplication *pending = NULL, *resolved = NULL;
    assert(binding_normalize_static(&binding_plan, NULL, expression, &pending));
    assert(pending->pending && pending->first->slot == NULL && pending->last->slot == NULL);
    BindingInterface *view = named_interface(NULL, 0);
    assert(binding_normalize_static(&binding_plan, view, expression, &resolved));
    assert(resolved != pending && !resolved->pending && resolved->first->slot != NULL);
    assert(pending->pending && pending->first->slot == NULL && pending->next == resolved);
    assert(pending->expression == resolved->expression);
    destroy_named_requests();
}

/** A nested value has inputs only and normalizes as its own application. The
 * outer request borrows the complete child rather than flattening its bindings.
 */
static void it_normalizes_nested_named_values_without_flattening_owners(void) {
    setup_named_requests("@...\n/test/fixture/v1.0.0/src/other\n .a < .family\n  .b < .value\n .out > .one\n");
    BindingInterface *view = named_interface(NULL, 0);
    BindingApplication *outer = NULL, *inner = NULL;
    const AstTypeExpression *child = expression->first->input;
    assert(binding_normalize_static(&binding_plan, view, expression, &outer));
    assert(binding_normalize_static(&binding_plan, view, child, &inner));
    assert(outer->input_count == 1 && outer->output_count == 1);
    assert(inner->input_count == 1 && inner->output_count == 0);
    assert(outer->first->syntax.static_binding->input == child);
    assert(inner->first->slot->original_ordinal == 1 && outer->first->slot->original_ordinal == 0);
    assert(outer->next == inner);
    destroy_named_requests();
}

/** Captures remove exact original identities from the frozen input view; a
 * supplied name cannot reopen a captured slot, even with its old ordinal.
 */
static void it_preserves_named_residuals_without_reopening_captured_inputs(void) {
    const char *cases[] = {"@...\n.partial\n.family\n .b < .value\n",
        "@...\n.partial\n.family\n .a < .value\n"};
    for (size_t index = 0; index < 2; index++) {
        setup_named_requests(cases[index]);
        const StaticSymbol *capture[] = {static_symbol_at(producer, STATIC_PARAMETER, 0)};
        BindingInterface *view = named_interface(capture, 1);
        assert(view->sequences[BINDING_STATIC_INPUT].count == 1);
        BindingApplication *application = NULL;
        bool success = binding_normalize_static(&binding_plan, view, expression, &application);
        assert(success == (index == 0));
        if (success) {
            assert(application->first->slot->original_ordinal == 1);
            assert(application->first->slot->sequence_ordinal == 0);
        } else { assert(application == NULL && binding_plan.status == BINDING_INVALID); }
        assert(producer->parameter_count == 2);
        destroy_named_requests();
    }
}
