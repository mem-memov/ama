/**
 * @file
 * @brief Compose catalog, structural grammar, diagnostics and typed native C fixtures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Count the actual structural nodes used by the parser composition assertions; no source-line
 * heuristic substitutes for the AST.
 */
static size_t node_count(AstBlock block) {
    size_t count = 0;
    for (AstNode *node = block.first; node != NULL; node = node->next) {
        count++;
    }
    return count;
}

/**
 * Assert that the component preserves source ranges through real file discovery and ast
 * construction. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_preserves_source_ranges_through_real_file_discovery_and_ast_construction(void) {
    setup_test();
    const ModuleSource *source = find_fixture("/test/fixture/v1.0.0/src/程序");
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_OK);
    FILE *output = fopen("generated/summary.txt", "wb");
    assert(output != NULL);
    int written = fprintf(output, "constants=%zu parameters=%zu results=%zu body=%zu\n",
        node_count(module.constants), node_count(module.parameters), node_count(module.results), node_count(module.body));
    assert(written >= 0);
    SourceSpan payload = module.constants.first->as.declaration.first->source->payload;
    size_t copied = fwrite(source_bytes(payload.source) + payload.start, 1, payload.end - payload.start, output);
    assert(copied == payload.end - payload.start);
    written = fputc('\n', output);
    assert(written != EOF);
    AstNode *call = module.body.first->next->next;
    assert(call->kind == AST_CALL);
    AstSource *input = call->as.call.first->source;
    assert(input->kind == AST_LITERAL_SOURCE && layout_text_is(input->payload, "原文 # |@!   "));
    written = fprintf(output, "inputs=%zu outputs=%zu\n", call->as.call.input_count, call->as.call.output_count);
    assert(written >= 0);
    int closed = fclose(output);
    assert(closed == 0);
    compare_output("generated/summary.txt", "expected.txt");
    destroy_test();
}

/**
 * Assert that the component reports a real parser error with original source and actionable
 * help. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_reports_a_real_parser_error_with_original_source_and_actionable_help(void) {
    setup_test();
    const ModuleSource *source = find_fixture("/test/fixture/v1.0.0/src/broken");
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_INVALID && module.error.expected == EXPECT_COPY_SOURCE);
    expect_error(&module.error, "expected-error.txt");
    destroy_test();
}

/**
 * Assert that the component keeps a native companion from activating an unfinished draft. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_keeps_a_native_companion_from_activating_an_unfinished_draft(void) {
    setup_test();
    const ModuleSource *source = find_fixture("/test/fixture/v1.0.0/src/draft");
    assert(!source->native_loaded && source->native_c == NULL && source->native_abi == NULL);
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_OK && module.sections.kind == MODULE_DRAFT && module.body.first == NULL);
    destroy_test();
}

/**
 * Assert that the component renders duplicate abi bindings with both source locations. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_renders_duplicate_abi_bindings_with_both_source_locations(void) {
    setup_test();
    const SourceText *source;
    SourceStatus loaded = source_manager_load(&catalog.sources, "project/packages/test/fixture/v1.0.0/src/bad.abi", &source);
    assert(loaded == SOURCE_OK);
    ParseStatus parsed = parser_abi_parse(&table, source);
    assert(parsed == PARSE_INVALID && table.error.expected == EXPECT_ABI_NEW_SYMBOL);
    expect_error(&table.error, "expected-abi-error.txt");
    destroy_test();
}

/**
 * Emit the test-only typed C bindings derived from the parsed native fixture, so C signature
 * compatibility is checked by compilation.
 */
static void write_typed_bindings(FILE *output) {
    const char *declarations[] = {
        "AmaStatus (*construct)(AMA_NATIVE(NativeValue) *) = ",
        "AmaStatus (*destroy)(AMA_NATIVE(NativeValue) *) = ",
        "AmaStatus (*literal)(AMA_NATIVE(NativeValue) *, AmaByteView) = ",
        "AmaStatus (*clone)(AMA_NATIVE(NativeValue) *, const AMA_NATIVE(NativeValue) *) = ",
        "AmaStatus (*condition)(const AMA_NATIVE(NativeValue) *, bool *) = ",
        "AmaStatus (*index_value_function)(const AMA_NATIVE(NativeValue) *, size_t *) = "
    };
    for (size_t operation = 0; operation < ABI_BINDING_COUNT; operation++) {
        int written = fputs(declarations[operation], output);
        assert(written != EOF);
        written = fputs("AMA_NATIVE(", output);
        assert(written != EOF);
        SourceSpan function = table.bindings[operation].function;
        size_t copied = fwrite(source_bytes(function.source) + function.start, 1, function.end - function.start, output);
        assert(copied == function.end - function.start);
        written = fputs(");\n", output);
        assert(written != EOF);
    }
}

/**
 * Emit one namespaced native fixture probe for actual compilation; equal local helper names must
 * remain separate across modules.
 */
static void write_module_probe(FILE *output, const ModuleSource *source, const IdentityName *name) {
    AstModule native;
    ast_module_init(&native, source_system_allocator());
    ParseStatus activated = parser_module_parse(&native, source->text);
    assert(activated == PARSE_OK && native.sections.kind == MODULE_NATIVE);
    ast_module_destroy(&native);
    ModuleStatus selected = module_catalog_load_native(&catalog, source);
    assert(selected == MODULE_OK && source->native_loaded);
    abi_table_destroy(&table);
    abi_table_init(&table, source_system_allocator());
    ParseStatus parsed = parser_abi_parse(&table, source->native_abi);
    assert(parsed == PARSE_OK);
    bool emitted = native_namespace_begin(output, name);
    assert(emitted);
    int written = fprintf(output, "#include \"../%s\"\nstatic int AMA_NATIVE(probe)(void) {\n", source->native_c);
    assert(written >= 0);
    write_typed_bindings(output);
    written = fputs("AMA_NATIVE(NativeValue) initial, value, copy; bool truth; size_t position;\n"
        "if (!ama_status_is_success(construct(&initial))) return 1;\n"
        "if (!ama_status_is_success(destroy(&initial))) return 7;\n"
        "if (!ama_status_is_success(literal(&value, (AmaByteView){(const unsigned char *)\"abc\", 3}))) return 2;\n"
        "if (!ama_status_is_success(clone(&copy, &value))) return 3;\n"
        "if (!ama_status_is_success(condition(&copy, &truth)) || !truth) return 4;\n"
        "if (!ama_status_is_success(index_value_function(&copy, &position)) || position != 3) return 5;\n"
        "if (!ama_status_is_success(destroy(&copy))) return 6;\n"
        "return !ama_status_is_success(destroy(&value)); }\n", output);
    assert(written != EOF);
    emitted = native_namespace_end(output);
    assert(emitted);
}

/**
 * Assert that the component isolates identical native names and compiles typed abi uses. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_isolates_identical_native_names_and_compiles_typed_abi_uses(void) {
    setup_test();
    const ModuleSource *source = find_fixture("/test/fixture/v1.0.0/src/native");
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_OK && module.sections.kind == MODULE_NATIVE);
    const ModuleSource *other;
    ModuleStatus found = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/other", &other);
    assert(found == MODULE_OK);
    const ModuleSource *modules[] = {source, other};
    const IdentityName *names[2];
    FILE *output = fopen("generated/bindings.c", "wb");
    assert(output != NULL);
    for (size_t index = 0; index < 2; index++) {
        QualifiedIdentity identity = {IDENTITY_NATIVE_NAMESPACE, modules[index]->logical_path, NULL, 0, {0}};
        IdentityStatus registered = identity_register(&identities, identity, &names[index]);
        assert(registered == IDENTITY_OK);
        write_module_probe(output, modules[index], names[index]);
    }
    const SourceText *probe;
    SourceStatus added = source_manager_add(&catalog.sources, "probe", (const unsigned char *)"probe", 5, &probe);
    assert(added == SOURCE_OK);
    int written = fputs("#ifdef AMA_NATIVE\n#error Leaked module context\n#endif\nint main(void) { return ", output);
    assert(written != EOF);
    for (size_t index = 0; index < 2; index++) {
        bool emitted = native_namespace_symbol(output, names[index], (SourceSpan){probe, 0, 5});
        assert(emitted);
        written = fputs(index == 0 ? "() || " : "(); }\n", output);
        assert(written != EOF);
    }
    int closed = fclose(output);
    assert(closed == 0);
    char *compiler[] = {TEST_REAL_COMPILER, "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror",
        "-I" TEST_ROOT "/src", "-finput-charset=UTF-8", "-fexec-charset=UTF-8",
        "generated/bindings.c", "-o", "generated/bindings", NULL};
    run_process(compiler);
    char *program[] = {"./generated/bindings", NULL};
    run_process(program);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_preserves_source_ranges_through_real_file_discovery_and_ast_construction();
    it_reports_a_real_parser_error_with_original_source_and_actionable_help();
    it_keeps_a_native_companion_from_activating_an_unfinished_draft();
    it_renders_duplicate_abi_bindings_with_both_source_locations();
    it_isolates_identical_native_names_and_compiles_typed_abi_uses();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
