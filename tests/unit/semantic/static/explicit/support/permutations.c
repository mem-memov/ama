/** @file
 * @brief Exhaust independent statement orders and distinguish source-defined first receipts.
 */

/** All six orders of one pair, one use and one receipt preserve pair authority
 * and the free-input set. No equation-solver scheduling participates in scope.
 */
static void it_keeps_pair_roles_for_every_three_statement_permutation(void) {
    const char *parts[] = {".declared\n.input\n", ".forward\n.declared\n",
        "/test/fixture/v1.0.0/src/other\n .out > .declared\n"};
    const size_t orders[][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    size_t size = strlen("@...\n") + strlen(parts[0]) + strlen(parts[1]) + strlen(parts[2]) + 1;
    for (size_t permutation = 0; permutation < 6; permutation++) {
        char *text = malloc(size);
        assert(text != NULL);
        memcpy(text, "@...\n", 5);
        size_t used = 5;
        for (size_t index = 0; index < 3; index++) {
            const char *part = parts[orders[permutation][index]];
            size_t length = strlen(part);
            memcpy(text + used, part, length);
            used += length;
        }
        assert(used + 1 == size);
        text[used] = 0;
        setup_static(text, public_graph_producer);
        free(text);
        assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
        const StaticModule *module = static_module_find(&inventory, project.entry);
        assert(module->parameter_count == 1 && module->result_count == 2);
        const StaticSymbol *input = static_symbol_at(module, STATIC_PARAMETER, 0);
        assert(layout_text_is(input->name, ".input") && input->category == STATIC_CATEGORY_DEFERRED);
        const StaticSymbol *declared = static_symbol_find(module, static_name_site(project.entry, ".declared"));
        assert(declared != NULL && declared->role == STATIC_RESULT && declared->receiver == NULL);
        assert(declared->declaration->kind == AST_STATIC_RESULT);
        assert(layout_text_is(declared->declaration->as.static_result.name, ".declared"));
        for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
            if (site->receiver != NULL) { assert(site->symbol == declared); }
        }
        destroy_static();
    }
}

/** Without a pair, source order really does select the defining receipt. The
 * second receipt is equality even when it chooses a different foreign output.
 */
static void it_uses_the_first_lexical_receipt_as_the_alias_definition(void) {
    const char *cases[] = {
        "@...\n/test/fixture/v1.0.0/src/other\n .out > .alias\n/test/fixture/v1.0.0/src/other\n .second > .alias\n",
        "@...\n/test/fixture/v1.0.0/src/other\n .second > .alias\n/test/fixture/v1.0.0/src/other\n .out > .alias\n"
    };
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], public_graph_producer);
        assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
        const StaticModule *module = static_module_find(&inventory, project.entry);
        const AstNode *first = project.entry->syntax.graph.first;
        const StaticSymbol *alias = static_symbol_at(module, STATIC_RESULT, 0);
        assert(module->result_count == 1 && module->parameter_count == 0);
        assert(alias != NULL && alias->role == STATIC_ALIAS && alias->declaration == first);
        assert(alias->receiver == first->as.static_projection->first);
        assert(alias->receiver != first->next->as.static_projection->first);
        assert(layout_text_is(alias->receiver->label, index == 0 ? ".out" : ".second"));
        destroy_static();
    }
}

/** A receipt preceding two equal pair definitions neither legalizes the
 * duplicate nor replaces the original pair header in its related diagnostic.
 */
static void it_reports_pair_duplicates_against_pair_headers_not_earlier_receipts(void) {
    setup_static("@...\n/test/fixture/v1.0.0/src/other\n .out > .same\n.same\n.input\n.same\n.input\n",
        public_graph_producer);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && !static_collect_explicit(&inventory, module));
    const AstNode *producer_node = project.entry->syntax.graph.first;
    const AstNode *first_pair = producer_node->next, *second_pair = first_pair->next;
    assert(inventory.status == STATIC_INVALID && diagnostic_valid(&inventory.error));
    assert(inventory.error.related_count == 1 && module->result_count == 1);
    assert(inventory.error.primary.span.start == second_pair->as.static_result.name.start);
    assert(inventory.error.related[0].location.span.start == first_pair->as.static_result.name.start);
    assert(inventory.error.related[0].location.span.start > producer_node->as.static_projection->first->receiver.start);
    destroy_static();
}
