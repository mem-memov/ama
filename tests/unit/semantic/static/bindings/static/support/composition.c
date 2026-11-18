/** @file
 * @brief Compose named, fixed-residual and nested applications against independently known producer identities.
 */

/** Foreign names select exact original producer fields; source binding order
 * changes neither the actual input snapshot nor its independent output order.
 */
static void it_maps_named_static_requests_to_exact_original_fields_in_arbitrary_order(void) {
    setup_requests(".other\n ...b < ...type\n ...a < ...type\n ...second > ...second received\n ...out > ...first received\n");
    BindingInterface *view = request_interface();
    BindingApplication *application = NULL;
    const AstTypeExpression *expression = request_head(".other")->expression;
    assert(binding_normalize_static(&binding_plan, view, expression, &application));
    assert(application->mode == BINDING_MODE_NAMED && application->input_count == 2 && application->output_count == 2);
    const BindingRequest *request = application->first;
    for (size_t index = 0; index < 4; index++) {
        assert(request != NULL && request->slot->original_ordinal == 1 - index % 2);
        assert(request->source_ordinal == index % 2 && !request->omitted && request->named);
        assert(request->label.source == project.entry->source->text);
        request = request->next;
    }
    assert(request == NULL && layout_text_is(view->sequences[BINDING_STATIC_INPUT].first->name.site, "...a"));
    destroy_binding();
}

/** A prepared partial constructor fixes a and promises only b; positional slot
 * zero therefore maps to original b, and a named request cannot reopen a.
 */
static void it_uses_fixed_residual_snapshots_without_reopening_captured_formals(void) {
    const char *bodies[] = {
        "@<\n...type\n...expected\n ...b\n@!\n...expected\n ... < ...type\n ... > ...received\n",
        "@<\n...type\n...expected\n ...b\n@!\n...expected\n ...b < ...type\n ...out > ...received\n",
        "@<\n...type\n...expected\n ...b\n@!\n...expected\n ...a < ...type\n ...out > ...received\n",
        "@<\n...type\n...expected\n ...b\n@!\n...expected\n ... < ...type\n ... <\n ... >\n"
    };
    for (size_t index = 0; index < 4; index++) {
        setup_binding(bodies[index], request_producer);
        const StaticModule *producer = binding_module(project_find(&project, ".other"));
        const StaticSymbol *fixed[] = {static_symbol_find(producer, static_name_site(producer->definition, "...a"), 0)};
        BindingInterface *view = NULL;
        assert(binding_source_interface(&binding_plan, producer, BINDING_MODULE_SURFACE, fixed, 1, &view));
        assert(view->sequences[BINDING_STATIC_INPUT].count == 1);
        BindingApplication *application = NULL;
        bool success = binding_normalize_static(&binding_plan, view, request_head("...expected")->expression, &application);
        assert(success == (index < 2));
        if (success) {
            assert(application->first->slot->original_ordinal == 1 && application->first->slot->sequence_ordinal == 0);
            assert(layout_text_is(application->first->slot->static_symbol->name, "...b"));
            assert(application->input_count == 1 && application->output_count == 1);
        } else { assert(application == NULL && binding_plan.status == BINDING_INVALID); }
        assert(view->sequences[BINDING_STATIC_INPUT].first->original_ordinal == 1 && producer->parameter_count == 2);
        destroy_binding();
    }
}

/** A positional parent and named child have independent mode/counter state.
 * The parent's first operand remains the complete child expression rather than
 * its selected fields or a flattened copy of that child's bindings.
 */
static void it_normalizes_nested_static_modes_without_flattening_child_requests(void) {
    setup_requests(".other\n ... < .main\n  ...expected < ...expected\n  ...type < ...type\n ... < ...type\n ... >\n");
    BindingInterface *parent_view = request_interface(), *child_view = NULL;
    assert(binding_source_interface(&binding_plan, binding_module(project.entry), BINDING_MODULE_SURFACE, NULL, 0, &child_view));
    const AstTypeExpression *parent = request_head(".other")->expression;
    const AstTypeExpression *child = parent->first->input;
    assert(child != NULL && layout_text_is(child->head, ".main"));
    BindingApplication *outer = NULL, *inner = NULL;
    assert(binding_normalize_static(&binding_plan, parent_view, parent, &outer));
    assert(binding_normalize_static(&binding_plan, child_view, child, &inner));
    assert(outer->mode == BINDING_MODE_POSITIONAL && outer->input_count == 2 && outer->output_count == 1);
    assert(inner->mode == BINDING_MODE_NAMED && inner->input_count == 2 && inner->output_count == 0);
    assert(outer->first->syntax.static_binding->input == child);
    assert(inner->first->slot->original_ordinal == 1 && inner->last->slot->original_ordinal == 0);
    assert(outer->first->next->slot->original_ordinal == 1);
    assert(outer->next == inner && parent->first->input == child && child->first->next == child->last);
    destroy_binding();
}
