/**
 * @file
 * @brief Bound parser depth by explicit source/arena ownership and fail every tree allocation.
 */

/** A long selected path creates heap frames and exact parent links, not a C call chain. */
static void it_parses_deep_result_paths_without_recursive_c_activation(void) {
    const size_t depth = 512;
    size_t capacity = (depth + 1) * (depth + 16) + 8;
    char *text = malloc(capacity);
    assert(text != NULL);
    size_t used = 0;
    int written = snprintf(text, capacity, "@!\n");
    assert(written > 0);
    used = (size_t)written;
    for (size_t index = 0; index <= depth; index++) {
        written = snprintf(text + used, capacity - used, "%*smethod !\n", (int)index, "");
        assert(written > 0 && (size_t)written < capacity - used);
        used += (size_t)written;
    }
    setup_tree();
    AstNode *operation = root(text);
    for (size_t index = 0; index < depth; index++) {
        assert(operation->as.call.output_count == 0);
        assert(operation->as.call.first_output != NULL && operation->as.call.first_output->next == NULL);
        AstNode *child = operation->as.call.first_output->operation;
        assert(child->as.call.enclosing_operation == operation);
        operation = child;
    }
    assert(operation->as.call.first_output == NULL);
    destroy_tree();
    free(text);
}

/** Each failure leaves the whole module invalid and all shared action/binding pointers under one destructible arena owner. */
static void it_unwinds_every_allocation_failure_in_a_result_tree_and_handler(void) {
    const char *text = "@!\nroot !\n < |input\n > receiver\n method& !\n  < &\n  child %\n   > value\n :\n  fallback !\n:\n >\n";
    setup_tree();
    ParseStatus status = parse_text(text);
    assert(status == PARSE_OK);
    size_t count = allocation_count;
    destroy_tree();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_tree();
        failure_at = failure;
        const SourceText *source = NULL;
        SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)text, strlen(text), &source);
        if (added == SOURCE_OK) {
            status = parser_module_parse(&module, source);
            assert(status == PARSE_OUT_OF_MEMORY && !module.valid);
        } else { assert(added == SOURCE_OUT_OF_MEMORY); }
        destroy_tree();
    }
}
