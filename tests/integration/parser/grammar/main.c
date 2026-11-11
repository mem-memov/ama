#include "../common.c"

static size_t node_count(AstBlock block) {
    size_t count = 0;
    for (AstNode *node = block.first; node != NULL; node = node->next) {
        count++;
    }
    return count;
}

static void it_preserves_source_ranges_through_real_file_discovery_and_ast_construction(void) {
    setup_test();
    const ModuleSource *source = find_fixture(".程序");
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
    written = fprintf(output, "inputs=%zu outputs=%zu\n", call->as.call.input_count, call->as.call.output_count);
    assert(written >= 0);
    int closed = fclose(output);
    assert(closed == 0);
    compare_output("generated/summary.txt", "expected.txt");
    destroy_test();
}

static void it_reports_a_real_parser_error_with_original_source_and_actionable_help(void) {
    setup_test();
    const ModuleSource *source = find_fixture(".broken");
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_INVALID && module.error.expected == EXPECT_COPY_SOURCE);
    expect_error(&module.error, "expected-error.txt");
    destroy_test();
}

static void it_keeps_a_native_companion_from_activating_an_unfinished_draft(void) {
    setup_test();
    const ModuleSource *source = find_fixture(".draft");
    assert(source->native_c != NULL);
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_OK && module.sections.kind == MODULE_DRAFT && module.body.first == NULL);
    destroy_test();
}

static void it_renders_duplicate_abi_bindings_with_both_source_locations(void) {
    setup_test();
    const SourceText *source;
    SourceStatus loaded = source_manager_load(&catalog.sources, "project/bad.abi", &source);
    assert(loaded == SOURCE_OK);
    ParseStatus parsed = parser_abi_parse(&table, source);
    assert(parsed == PARSE_INVALID && table.error.expected == EXPECT_ABI_NEW_SYMBOL);
    expect_error(&table.error, "expected-abi-error.txt");
    destroy_test();
}

static void write_typed_bindings(FILE *output) {
    const char *declarations[] = {
        "AmaStatus (*construct)(NativeValue *) = ", "AmaStatus (*destroy)(NativeValue *) = ",
        "AmaStatus (*literal)(NativeValue *, AmaByteView) = ",
        "AmaStatus (*clone)(NativeValue *, const NativeValue *) = ",
        "AmaStatus (*condition)(const NativeValue *, bool *) = ",
        "AmaStatus (*index_value_function)(const NativeValue *, size_t *) = "
    };
    for (size_t operation = 0; operation < ABI_BINDING_COUNT; operation++) {
        int written = fputs(declarations[operation], output);
        assert(written != EOF);
        SourceSpan function = table.bindings[operation].function;
        size_t copied = fwrite(source_bytes(function.source) + function.start, 1, function.end - function.start, output);
        assert(copied == function.end - function.start);
        written = fputs(";\n", output);
        assert(written != EOF);
    }
}

static void it_compiles_and_runs_typed_uses_of_preserved_native_identifier_spellings(void) {
    setup_test();
    const ModuleSource *source = find_fixture(".native");
    ParseStatus parsed = parser_module_parse(&module, source->text);
    assert(parsed == PARSE_OK && module.sections.kind == MODULE_NATIVE);
    parsed = parser_abi_parse(&table, source->native_abi);
    assert(parsed == PARSE_OK);
    FILE *output = fopen("generated/bindings.c", "wb");
    assert(output != NULL);
    int written = fputs("#include \"../project/native.c\"\n", output);
    assert(written != EOF);
    write_typed_bindings(output);
    written = fputs("int main(void) { NativeValue value, copy; bool truth; size_t position;\n"
        "if (!ama_status_is_success(construct(&value))) return 1;\n"
        "if (!ama_status_is_success(literal(&value, (AmaByteView){(const unsigned char *)\"abc\", 3}))) return 2;\n"
        "if (!ama_status_is_success(clone(&copy, &value))) return 3;\n"
        "if (!ama_status_is_success(condition(&copy, &truth)) || !truth) return 4;\n"
        "if (!ama_status_is_success(index_value_function(&copy, &position)) || position != 3) return 5;\n"
        "if (!ama_status_is_success(destroy(&copy))) return 6;\n"
        "return !ama_status_is_success(destroy(&value)); }\n", output);
    assert(written != EOF);
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

void run(void) {
    it_preserves_source_ranges_through_real_file_discovery_and_ast_construction();
    it_reports_a_real_parser_error_with_original_source_and_actionable_help();
    it_keeps_a_native_companion_from_activating_an_unfinished_draft();
    it_renders_duplicate_abi_bindings_with_both_source_locations();
    it_compiles_and_runs_typed_uses_of_preserved_native_identifier_spellings();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
