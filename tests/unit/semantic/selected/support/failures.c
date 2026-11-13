/** @file
 * @brief Fail every semantic allocation in caller operands, selected targets, indices, nested handlers and ordered action publication.
 */
/** A complete selected tree never publishes resolved after any measured allocation fails; all partial records remain owned. */
static void it_unwinds_each_allocation_in_selected_operations_and_action_links(void) {
    prepare_selection();
    const char *text = "@!\nproducer\n.producer\nindex\n.value\n |0\nfallback\n.worker\n"
        "producer !\n method !\n  < |bytes\n  > result\n :\n  fallback !\n   > result\n"
        " methods[index] %\n  > result\n marker > last\n";
    SemanticStatus status = resolve_selected(text);
    assert(status == SEMANTIC_OK);
    size_t count = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        file("generated/program/main", text);
        setup_semantic();
        status = load_semantic("generated/program");
        assert(status == SEMANTIC_OK);
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OK);
        if (failure <= allocation_count) { destroy_semantic(); continue; }
        failure_at = failure;
        status = semantic_resolve(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.resolved && diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
