/**
 * @file
 * @brief Verify current graph mode refusals, unbounded iterative nesting and allocation teardown.
 */

/** Ordinary interfaces reject standalone static entries; graphs reject runtime bodies and output-bearing values. */
static void it_rejects_old_static_forms_and_invalid_graph_modes(void) {
    const char *invalid[] = {
        "@<\n.type\n@!\n", "@>\n.result\n//integer\n@!\n",
        "@...\n.result\n", "@...\n/test/fixture/v1.0.0/src/factory\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n .input < //integer\n",
        "@...\n.result\n/test/fixture/v1.0.0/src/factory\n .out > .alias\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n .input < /test/fixture/v1.0.0/src/nested\n  .out > .alias\n .out > .result\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n .out > .result\n .input < //integer\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n ... > .result\n", "@...\n.result\n/test/fixture/v1.0.0/src/box\n ... < //integer\n",
        "@...\n/test/fixture/v1.0.0/src/factory\n .out >\n", "@...\n.result\n/test/fixture/v1.0.0/src/box\n .input <\n",
        "@...\n.result\n//integer !\n", "@...\n.result\n|0\n",
        "@...\nworker !\n", "@...\ncondition\n{\n}\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_tree();
        ParseStatus status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Finite source nesting is represented by explicit frames, with no compiler depth cutoff. */
static void it_parses_deep_input_only_values_without_c_recursion(void) {
    FILE *stream = tmpfile();
    assert(stream != NULL);
    int written = fputs("@...\n.deep\n/test/fixture/v1.0.0/src/box\n", stream);
    assert(written >= 0);
    const size_t depth = 1024;
    for (size_t level = 1; level <= depth; level++) {
        for (size_t space = 0; space < level; space++) {
            int byte = fputc(' ', stream);
            assert(byte != EOF);
        }
        written = fputs(level == depth ? ".element < //integer\n" : ".element < /test/fixture/v1.0.0/src/box\n", stream);
        assert(written >= 0);
    }
    int flushed = fflush(stream), sought = fseek(stream, 0, SEEK_END);
    assert(flushed == 0 && sought == 0);
    long length = ftell(stream);
    assert(length > 0);
    char *text = malloc((size_t)length + 1);
    assert(text != NULL);
    sought = fseek(stream, 0, SEEK_SET);
    assert(sought == 0);
    size_t read = fread(text, 1, (size_t)length, stream);
    assert(read == (size_t)length);
    text[length] = '\0';
    int closed = fclose(stream);
    assert(closed == 0);
    setup_tree();
    ParseStatus status = parse_text(text);
    free(text);
    assert(status == PARSE_OK && module.graph.first->kind == AST_STATIC_RESULT);
    const AstTypeExpression *expression = module.graph.first->as.static_result.expression;
    for (size_t level = 0; level < depth; level++) {
        assert(expression->first != NULL && expression->first == expression->last);
        assert(expression->first->kind == AST_STATIC_INPUT);
        expression = expression->first->input;
    }
    assert(expression->first == NULL && layout_text_is(expression->head, "//integer"));
    destroy_tree();
}

/** Every observed backing allocation can refuse; partial graph/type owners remain safely destructible. */
static void it_unwinds_every_current_graph_and_selector_parse_allocation_failure(void) {
    const char *sources_text[] = {
        "@...\n.ready\n//integer\n/test/fixture/v1.0.0/src/factory\n .input < /test/fixture/v1.0.0/src/box\n  .element < .free\n .first > .one\n .second > .two\n",
        "@<\nargument\n.type\n@>\nresult\n/test/fixture/v1.0.0/src/assembly\n .ready\n@!\n/test/fixture/v1.0.0/src/assembly\n .ready\n zero >> output\n"
    };
    for (size_t example = 0; example < sizeof(sources_text) / sizeof(*sources_text); example++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_tree();
            const SourceText *source;
            SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)sources_text[example],
                strlen(sources_text[example]), &source);
            assert(added == SOURCE_OK);
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            ParseStatus status = parser_module_parse(&module, source);
            if (!failure) {
                assert(status == PARSE_OK && module.valid);
                total = allocation_count - before;
                assert(total != 0);
            } else {
                assert(status == PARSE_OUT_OF_MEMORY && !module.valid);
                assert(module.error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
            }
            failure_at = 0;
            destroy_tree();
        }
    }
}
