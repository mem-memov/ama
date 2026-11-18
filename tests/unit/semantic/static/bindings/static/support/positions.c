/** @file
 * @brief Check actual static positions and immutable unknown-to-known request results.
 */
/** Positional requests use actual producer a,b order despite caller's expected
 * b,a order. Empty outputs consume their own positions without alias creation.
 */
static void it_uses_actual_static_positions_without_renumbering_or_expected_signature_order(void) {
    setup_requests("...expected\n ... < ...type\n ... < ...type\n ... > ...received\n ... >\n");
    BindingInterface *view = request_interface();
    const StaticModule *caller = binding_module(project.entry);
    const StaticSymbol *expected = static_symbol_find(caller, static_name_site(project.entry, "...expected"), 0);
    assert(expected != NULL && layout_text_is(expected->signature->first->name, "...b"));
    assert(layout_text_is(view->sequences[BINDING_STATIC_INPUT].first->name.site, "...a"));
    const BindingSlot *first = view->sequences[BINDING_STATIC_INPUT].first;
    BindingApplication *application = NULL;
    const AstTypeExpression *expression = request_head("...expected")->expression;
    assert(binding_normalize_static(&binding_plan, view, expression, &application));
    assert(!application->pending && application->input_count == 2 && application->output_count == 2);
    assert(application->mode == BINDING_MODE_POSITIONAL && application->expression == expression);
    const BindingRequest *request = application->first;
    const AstStaticBinding *raw = expression->first;
    for (size_t ordinal = 0; ordinal < 4; ordinal++) {
        assert(request != NULL && request->syntax.static_binding == raw);
        assert(request->source_ordinal == ordinal % 2 && request->slot->original_ordinal == ordinal % 2);
        assert(request->direction == (ordinal < 2 ? BINDING_STATIC_INPUT : BINDING_STATIC_OUTPUT));
        assert(request->omitted == (ordinal == 3));
        request = request->next;
        raw = raw->next;
    }
    assert(request == NULL && view->sequences[BINDING_STATIC_INPUT].first == first);
    assert(first->next->original_ordinal == 1 && view->sequences[BINDING_STATIC_INPUT].count == 2);
    destroy_binding();
}

/** Missing actual origin never borrows expected signature order. Re-normalizing
 * with independent origin evidence creates a new immutable result, leaving the
 * earlier pending requests untouched and their raw source pointers stable.
 */
static void it_retains_unknown_static_origins_until_independent_actual_metadata_is_supplied(void) {
    setup_requests("...expected\n ... < ...type\n ... < ...type\n ... > ...received\n");
    const AstTypeExpression *expression = request_head("...expected")->expression;
    BindingApplication *pending = NULL;
    assert(binding_normalize_static(&binding_plan, NULL, expression, &pending));
    assert(pending->pending && pending->interface == NULL && pending->input_count == 2 && pending->output_count == 1);
    for (const BindingRequest *request = pending->first; request != NULL; request = request->next) { assert(request->slot == NULL); }
    BindingInterface *view = request_interface();
    BindingApplication *resolved = NULL;
    assert(binding_normalize_static(&binding_plan, view, expression, &resolved));
    assert(resolved != pending && !resolved->pending && pending->pending && pending->first->slot == NULL);
    assert(pending->expression == resolved->expression && pending->next == resolved);
    assert(resolved->first->slot->static_symbol->ordinal == 0);
    destroy_binding();
}
