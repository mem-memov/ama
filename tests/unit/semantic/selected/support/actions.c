/** @file
 * @brief Inspect resolved parent-result identities, interleaved action links and caller-owned handler/operand contexts.
 */
/** Siblings retain the same declared field identity on repeated selection; no target copy or caller variable replaces it. */
static void it_retains_selected_field_identity_and_ordered_sibling_actions_without_cloning(void) {
    prepare_selection();
    SemanticStatus status = resolve_selected("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nmethod\n/test/fixture/v1.0.0/src/value\nindex\n/test/fixture/v1.0.0/src/value\n |0\n"
        "producer\n method !\n  < method\n  > first\n alias& %\n  > second\n marker >> last\n"
        " methods[index] !\n  output > first\n references&[index]\n  output >> second\n"
        " method\n  output >> first\n");
    if (status != SEMANTIC_OK) { fprintf(stderr, "Selection: %s\n", semantic.error.message); }
    assert(status == SEMANTIC_OK && semantic.resolved);
    SemanticStatement *parent = selected_call("producer", 0);
    assert(parent != NULL && parent->enclosing_operation == NULL && !parent->namespace_type->cloneable);
    const char *names[] = {"method", "alias", "methods", "references", "method"};
    SemanticOutputAction *action = parent->actions;
    SemanticSymbol *first_method = NULL;
    SemanticOutputAction *ordinary = NULL;
    SemanticOutputAction *last = NULL;
    for (size_t index = 0; index < 5; index++) {
        if (index == 2) {
            assert(action != NULL && action->binding != NULL && action->operation == NULL);
            ordinary = action;
            action = action->next;
        }
        assert(action != NULL && action->binding == NULL && action->operation != NULL);
        SemanticStatement *child = action->operation;
        assert(child->enclosing_operation == parent && child->access->symbol->role == SYMBOL_RESULT);
        assert(child->access->symbol->owner == parent->namespace_type && !child->namespace_type->cloneable);
        assert(layout_text_is(child->access->symbol->name, names[index]));
        assert(child->access->reference == (index == 1 || index == 3) && !child->access->list);
        if (index == 0) {
            first_method = child->access->symbol;
            assert(child->bindings->source->access->symbol->owner == semantic_entry());
            assert(layout_text_is(child->bindings->source->access->symbol->name, "method"));
        }
        if (index == 4) { assert(child->access->symbol == first_method); }
        if (index == 2 || index == 3) {
            assert(child->access->index != NULL && child->access->index->symbol->owner == semantic_entry());
            assert(layout_text_is(child->access->index->symbol->name, "index"));
        }
        assert(child->actions != NULL && child->actions->binding->receiver->symbol->owner == semantic_entry());
        last = action;
        action = action->next;
    }
    assert(action == NULL && ordinary != NULL);
    assert(layout_text_is(ordinary->binding->field->name, "marker"));
    assert(parent->bindings == ordinary->binding && parent->last_action == last);
    destroy_semantic();
}
/** A colon body contains new root calls whose names resolve in the original caller, even when nested beneath selected results. */
static void it_starts_handler_calls_in_the_caller_instead_of_the_selected_interface(void) {
    prepare_selection();
    SemanticStatus status = resolve_selected("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nfallback\n/test/fixture/v1.0.0/src/worker\n"
        "producer\n method !\n  > result\n :\n  fallback !\n   > result\n:\n fallback !\n  > result\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *parent = selected_call("producer", 0);
    SemanticStatement *method = selected_call("method", 0);
    assert(method->enclosing_operation == parent && parent->actions->operation == method);
    for (size_t index = 0; index < 2; index++) {
        SemanticStatement *handler = selected_call("fallback", index);
        assert(handler != NULL && handler->enclosing_operation == NULL);
        assert(handler->syntax->as.call.enclosing_operation == NULL);
        assert(handler->access->symbol->owner == semantic_entry());
    }
    destroy_semantic();
}
