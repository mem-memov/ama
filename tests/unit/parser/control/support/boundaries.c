/** @file
 * @brief Reject nearby malformed shapes and exhaust finite frame/allocation boundaries.
 */
/** No declaration lookup or indentation repair makes these malformed headers legal. */
static void it_rejects_nonadjacent_markers_nonlinear_headers_and_reopened_regions(void) {
    const char *cases[] = {
        "@!\nready\n\n?\n", "@!\nready\n # wrong region\n?\n",
        "@!\nready\n:\n?\n", "@!\nready\n:\n:\n",
        "@!\nready&\n?\n", "@!\nlist[]\n{\n}\n",
        "@!\nlist[index[item&]]\n{\n}\n", "@!\nlist[item&[]]\n{\n}\n",
        "@!\nready\n{\n# closing gap forbidden\n}\n",
        "@!\nready\n?\n# body closed\n work !\n",
        "@!\nroot !\n > copy\n condition\n {\n }\n",
        "@!\nroot !\n first !\n condition\n {\n }\n",
        "@!\nroot !\n condition\n {\n }\n second !\n",
        "@!\nroot %\n < input\n condition\n {\n }\n",
        "@!\nroot !\n condition&\n {\n }\n",
        "@!\nroot !\n condition\n\n {\n }\n",
        "@!\nroot !\n condition\n {\n }\n after !\n",
        "@!\nroot !\n condition\n {\n }\n  misplaced !\n",
        "@!\n^\n", "@!\n!\n", "@!\n:\n"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        setup_tree();
        ParseStatus status = parse_text(cases[index]);
        if (status == PARSE_OK) { fprintf(stderr, "Unexpected valid control:\n%s", cases[index]); }
        assert(status == PARSE_INVALID && !module.valid);
        assert(diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}
/** Iterative frames own finite depth; each allocation failure invalidates the complete tree and releases all resources. */
static void it_handles_deep_control_frames_and_all_allocation_failures(void) {
    const size_t depth = 256;
    size_t capacity = depth * depth * 3 + depth * 64;
    char *text = malloc(capacity);
    assert(text != NULL);
    size_t used = (size_t)snprintf(text, capacity, "@!\n");
    for (size_t index = 0; index < depth; index++) {
        int written = snprintf(text + used, capacity - used, "%*sready\n%*s{\n", (int)index, "", (int)index, "");
        assert(written > 0 && (size_t)written < capacity - used);
        used += (size_t)written;
    }
    for (size_t index = depth; index != 0; index--) {
        int written = snprintf(text + used, capacity - used, "%*s}\n", (int)(index - 1), "");
        assert(written > 0 && (size_t)written < capacity - used);
        used += (size_t)written;
    }
    setup_tree();
    AstNode *node = parsed(text);
    for (size_t index = 0; index < depth; index++) {
        assert(node->kind == AST_LOOP);
        node = node->as.control.body.first;
    }
    assert(node == NULL && module.warnings != NULL && module.warnings->next == NULL);
    destroy_tree();
    free(text);
    const char *fixture = "@!\nready\n?\n:\na !\n child %\n  condition\n  {\n   list&[item&]\n   {\n   }\n  }\n :\n  >\n";
    setup_tree();
    parsed(fixture);
    size_t total = allocation_count;
    destroy_tree();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_tree();
        failure_at = failure;
        const SourceText *source = NULL;
        SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)fixture, strlen(fixture), &source);
        if (added == SOURCE_OK) {
            ParseStatus status = parser_module_parse(&module, source);
            assert(status == PARSE_OUT_OF_MEMORY && !module.valid);
        } else { assert(added == SOURCE_OUT_OF_MEMORY); }
        destroy_tree();
    }
}
