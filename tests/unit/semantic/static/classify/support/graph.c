/** @file
 * @brief Check exact graph input set complements, nested source order and unconstrained metadata.
 */

/** Foreign key/out labels precede a genuine local key use. The received name is
 * public despite appearing as a source before its first defining receipt.
 */
static const char *graph_input_source = "@...\n.use received\n.received\n/test/fixture/v1.0.0/src/other\n"
    " .key < .forwarded only\n .out > .received\n"
    ".wrapped\n.family\n .key < .key\n  .nested < .element\n";

/** Classification uses complete local source heads minus every public binder,
 * not foreign labels or the first available actual. Even applied graph inputs
 * retain Deferred actual metadata until the separate use-requirement stage.
 */
static void it_collects_graph_inputs_from_nested_sources_after_all_public_names(void) {
    setup_static(graph_input_source, "@...\n.out\n.key\n");
    StaticModule *module = prepare();
    assert(static_classify_names(&inventory, module));
    assert(module->parameter_count == 4 && module->result_count == 3);
    const char *names[] = {".forwarded only", ".family", ".key", ".element"};
    size_t previous = 0;
    for (size_t ordinal = 0; ordinal < 4; ordinal++) {
        const StaticSymbol *symbol = static_symbol_at(module, STATIC_PARAMETER, ordinal);
        assert(symbol != NULL && layout_text_is(symbol->name, names[ordinal]));
        assert(symbol->category == STATIC_CATEGORY_DEFERRED && symbol->signature == NULL);
        assert(symbol->site.start > previous && symbol->receiver == NULL);
        previous = symbol->site.start;
    }
    const StaticSymbol *key = static_symbol_at(module, STATIC_PARAMETER, 2);
    assert(key->site.start > static_name_site(project.entry, ".key").start);
    assert(static_symbol_find(module, static_name_site(project.entry, ".out")) == NULL);
    assert(static_symbol_find(module, static_name_site(project.entry, ".nested")) == NULL);
    const StaticSymbol *received = static_symbol_find(module, static_name_site(project.entry, ".received"));
    assert(received != NULL && received->role == STATIC_ALIAS);
    assert(static_bind_occurrences(&inventory, module));
    for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver != NULL) { assert(site->symbol->role != STATIC_PARAMETER); }
    }
    destroy_static();
}

/** Matching text in separate graph definitions has independent input/output
 * identities. A source span is a bounded name query, never a scope capture.
 */
static void it_keeps_equal_graph_input_spellings_in_independent_module_scopes(void) {
    const char *text = "@...\n.out\n.same\n";
    setup_static(text, text);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *left = static_module_find(&inventory, project.entry);
    const StaticModule *right = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/other"));
    const StaticSymbol *a = static_symbol_at(left, STATIC_PARAMETER, 0);
    const StaticSymbol *b = static_symbol_at(right, STATIC_PARAMETER, 0);
    assert(a != NULL && b != NULL && a != b && a->owner != b->owner);
    assert(static_same_name(a->name, b->name) && a->name.source != b->name.source);
    assert(static_symbol_find(left, b->name) == a && static_symbol_find(right, a->name) == b);
    assert(a->category == STATIC_CATEGORY_DEFERRED && b->category == STATIC_CATEGORY_DEFERRED);
    assert(left->parameter_count == 1 && right->parameter_count == 1);
    assert(left->result_count == 1 && right->result_count == 1);
    destroy_static();
}

/** Each actual free-input allocation can refuse independently of the already
 * fixed public table. Failure cannot change output roles, count or definition.
 */
static void it_unwinds_every_graph_input_classification_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(graph_input_source, "@...\n.out\n.key\n");
        StaticModule *module = prepare();
        const StaticSymbol *received = static_symbol_find(module, static_name_site(project.entry, ".received"));
        const AstStaticBinding *definition = received->receiver;
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = static_classify_names(&inventory, module);
        if (failure == 0) { assert(success); total = allocation_count - before; assert(total == 4); }
        else {
            assert(!success && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid);
            before = allocation_count;
            assert(!static_classify_names(&inventory, module) && allocation_count == before);
        }
        assert(module->result_count == 3 && received->receiver == definition && received->role == STATIC_ALIAS);
        destroy_static();
    }
}
