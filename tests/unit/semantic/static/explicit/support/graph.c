/** @file
 * @brief Verify whole-graph pair priority, first receipts, forward aliases and public snapshots.
 */

/** A receipt precedes its pair declaration. Another receiver defines an alias;
 * a later receipt into that alias must not replace its defining producer edge.
 */
static const char *public_graph_source = "@...\n.other\n"
    " ...out > ...declared\n ...second > ...alias\n"
    "...forward\n...alias\n...declared\n...input\n"
    ".other\n ...out > ...alias\n";
static const char *public_graph_producer = "@...\n...out\n...producer input\n"
    "...second\n...producer input\n";

/** Public identity/order follows definitions, not allocation order, the first
 * equality receipt or metadata solving. The primitive pass creates no inputs.
 */
static void it_collects_pairs_before_receipts_and_retains_first_alias_definition(void) {
    setup_static(public_graph_source, public_graph_producer);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module));
    assert(module->parameter_count == 0 && module->result_count == 3);
    const AstNode *first = project.entry->syntax.graph.first;
    const AstNode *forward = first->next, *declared = forward->next, *later = declared->next;
    const StaticSymbol *pair = static_symbol_find(module, declared->as.static_result.name);
    const AstStaticBinding *original_receipt = first->as.static_projection->last;
    const StaticSymbol *alias = static_symbol_find(module, original_receipt->receiver);
    assert(pair != NULL && pair->role == STATIC_RESULT && pair->declaration == declared && pair->receiver == NULL);
    assert(alias != NULL && alias->role == STATIC_ALIAS && alias->declaration == first);
    assert(alias->receiver == original_receipt && alias->receiver != later->as.static_projection->first);
    assert(pair->site.start > first->as.static_projection->first->receiver.start);
    assert(module->first->declaration == forward && module->last == alias);
    assert(static_symbol_at(module, STATIC_RESULT, 0) == alias);
    assert(static_symbol_at(module, STATIC_RESULT, 1)->declaration == forward);
    assert(static_symbol_at(module, STATIC_RESULT, 2) == pair);
    assert(static_symbol_at(module, STATIC_RESULT, 3) == NULL);
    assert(static_symbol_at(module, STATIC_ALIAS, 0) == NULL);
    size_t before = allocation_count;
    static_order_symbols(module, STATIC_RESULT);
    assert(allocation_count == before && alias->ordinal == 0 && pair->ordinal == 2);
    destroy_static();
}

/** The completed lexical inventory exposes every pair/alias in an actual frozen
 * output interface. Foreign labels create no input and runtime fields stay empty.
 */
static void it_exposes_every_public_alias_in_the_actual_source_interface(void) {
    setup_static(public_graph_source, public_graph_producer);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->parameter_count == 1 && module->result_count == 3);
    const StaticSymbol *input = static_symbol_at(module, STATIC_PARAMETER, 0);
    assert(layout_text_is(input->name, "...input") && input->category == STATIC_CATEGORY_DEFERRED);
    BindingPlan plan;
    binding_plan_init(&plan, sources.allocator);
    BindingInterface *view = NULL;
    assert(binding_source_interface(&plan, module, BINDING_MODULE_SURFACE, NULL, 0, &view));
    assert(view->sequences[BINDING_STATIC_OUTPUT].count == 3);
    assert(view->sequences[BINDING_STATIC_INPUT].count == 1);
    assert(view->sequences[BINDING_RUNTIME_INPUT].count == 0 && view->sequences[BINDING_RUNTIME_OUTPUT].count == 0);
    const char *names[] = {"...alias", "...forward", "...declared"};
    const BindingSlot *slot = view->sequences[BINDING_STATIC_OUTPUT].first;
    for (size_t index = 0; index < 3; index++) {
        assert(slot != NULL && layout_text_is(slot->name.site, names[index]));
        assert(slot->static_symbol == static_symbol_at(module, STATIC_RESULT, index));
        assert(slot->original_ordinal == index && slot->sequence_ordinal == index);
        slot = slot->next;
    }
    assert(slot == NULL);
    binding_plan_destroy(&plan);
    destroy_static();
}

/** An alias used before its first receipt is still an output, never a guessed
 * external parameter. Type/value closure is deliberately not claimed here.
 */
static void it_binds_forward_alias_uses_without_creating_external_inputs(void) {
    setup_static("@...\n...forward\n...alias\n.other\n ...out > ...alias\n", public_graph_producer);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->parameter_count == 0 && module->result_count == 2);
    const StaticOccurrence *site = module->first_occurrence;
    assert(site != NULL && site->symbol != NULL && site->symbol->role == STATIC_ALIAS);
    assert(site->site.start < site->symbol->site.start);
    assert(site->symbol->receiver != NULL && site->symbol->category == STATIC_CATEGORY_DEFERRED);
    destroy_static();
}

/** Every allocation used by pair/first-receipt collection is refused once.
 * Repeated receipts reuse records; a prior failure forbids later helper work.
 */
static void it_unwinds_every_public_receipt_collection_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(public_graph_source, public_graph_producer);
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = static_collect_explicit(&inventory, module);
        if (failure == 0) { assert(success); total = allocation_count - before; assert(total != 0); }
        else {
            assert(!success && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid);
            before = allocation_count;
            assert(!static_collect_receivers(&inventory, module) && allocation_count == before);
            assert(diagnostic_valid(&inventory.error));
        }
        destroy_static();
    }
}
