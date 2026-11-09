#include "../common.c"

static void declare_add(void) {
    Token zero = {.type = TOKEN_LITERAL, .lexeme = "0"};
    Token path = {.type = TOKEN_MODULE_PATH, .lexeme = ".math.add"};
    Symbol* left = add_declarative_symbol(symbols, "math/add", "left", SYMBOL_PARAMETER, zero, &error);
    Symbol* right = add_declarative_symbol(symbols, "math/add", "right", SYMBOL_PARAMETER, zero, &error);
    Symbol* sum = add_declarative_symbol(symbols, "math/add", "sum", SYMBOL_RESULT, zero, &error);
    Symbol* alias = add_declarative_symbol(symbols, "main", "add", SYMBOL_MODULE_ALIAS, path, &error);
    int node = graph_add_node(graph, "math/add", &error);
    assert(left && right && sum && alias && node >= 0);
}

static void it_copies_parameters_before_execution_and_results_after(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add!\n < 5\n < 7\n > result\n");
    assert(parsed);
    assert_stream_equals(output, "    ama_instance_a97b938813d0c822.ama_field_bacbc74569737656 = 0;\n    ama_instance_a97b938813d0c822.ama_field_96658b48fa2d5d94 = 0;\n    ama_instance_a97b938813d0c822.ama_field_37f96d0237c09c53 = 0;\n"
        "    ama_instance_a97b938813d0c822.ama_field_bacbc74569737656 = 5;\n    ama_instance_a97b938813d0c822.ama_field_96658b48fa2d5d94 = 7;\n    ama_call_a97b938813d0c822(&ama_instance_a97b938813d0c822);\n    state->ama_field_461586e9099c6ddb = ama_instance_a97b938813d0c822.ama_field_37f96d0237c09c53;\n");
    destroy_test();
}

static void it_accepts_named_parameters_and_results(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add!\n right < 7\n left < 5\n sum > result\n");
    assert(parsed && lookup_symbol(symbols, "main", "result") != NULL);
    assert(lookup_symbol(symbols, "main", "left") == NULL);
    assert(lookup_symbol(symbols, "main", "right") == NULL);
    assert(lookup_symbol(symbols, "main", "sum") == NULL);
    destroy_test();
}

static void it_rejects_mixed_argument_modes(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add!\n < 5\n right < 7\n");
    assert(!parsed && strstr(error.message, "cannot mix") != NULL);
    destroy_test();
}

static void it_rejects_parameters_after_results(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add!\n > result\n < 5\n");
    assert(!parsed && strstr(error.message, "parameter after result") != NULL);
    destroy_test();
}

static void it_reads_a_local_before_its_call_result_assignment(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add!\n < accumulated\n < 1\n > accumulated\n");
    assert(parsed);
    Symbol* accumulated = lookup_symbol(symbols, "main", "accumulated");
    assert(accumulated && accumulated->kind == SYMBOL_LOCAL_VARIABLE);
    assert(symbols->symbol_count == 5);
    destroy_test();
}

static void it_maps_delayed_execution_to_the_call_instead_of_its_last_argument(void) {
    setup_test();
    declare_add();
    emitter.source_mapping_enabled = true;
    bool parsed = parse_text_at("add!\n < 5\n < 7\n > answer\n", "caller.ama");
    assert(parsed);
    rewind(output);
    char text[8192];
    size_t length = fread(text, 1, sizeof(text) - 1, output);
    assert(!ferror(output));
    text[length] = '\0';
    assert(strstr(text, "#line 2 \"caller.ama\"\n") != NULL);
    assert(strstr(text, "#line 3 \"caller.ama\"\n") != NULL);
    const char* argument = strstr(text, "#line 3");
    const char* execution = strstr(argument, "#line 1 \"caller.ama\"\n");
    const char* result = strstr(text, "#line 4 \"caller.ama\"\n");
    assert(execution && result && argument < execution && execution < result);
    Symbol* answer = lookup_symbol(symbols, "main", "answer");
    assert(answer && answer->declaration.line == 4 && answer->declaration.column == 4);
    assert(strcmp(answer->declaration.path, "caller.ama") == 0);
    destroy_test();
}

static void it_parses_calls_without_any_alias_declaration(void) {
    setup_test();
    bool parsed = parse_syntax_at("unknown! # comment\n", "calls.ama");
    assert(parsed && syntax.declarations == NULL && syntax.body.first->kind == AST_CALL);
    assert(strcmp(syntax.body.first->as.call.target.text, "unknown") == 0);
    assert(syntax.body.first->as.call.arguments == NULL && syntax.body.first->location.column == 1);
    bool validated = validate_module_body(&syntax, symbols, graph, "main", &error);
    assert(!validated && strstr(error.message, "unknown module alias 'unknown'") != NULL);
    destroy_test();
}

static void it_requires_an_adjacent_marker_and_separate_argument_lines(void) {
    setup_test();
    const char* invalid[] = {"worker !\n", "worker\n!\n", "worker! < value\n",
        "worker!!\n", "!\n", "worker\n", "worker\n < value\n"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        ast_destroy(&syntax);
        error_clear(&error);
        bool parsed = parse_syntax_at(invalid[index], "calls.ama");
        assert(!parsed && error.error_code == ERROR_INVALID);
    }
    destroy_test();
}

static void it_executes_two_explicit_zero_argument_calls_in_order(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add!\nadd!\n");
    assert(parsed && syntax.body.first->kind == AST_CALL && syntax.body.first->next->kind == AST_CALL);
    assert(syntax.body.first->as.call.arguments == NULL && syntax.body.first->next->as.call.arguments == NULL);
    destroy_test();
}

void run(void) {
    it_parses_calls_without_any_alias_declaration();
    it_requires_an_adjacent_marker_and_separate_argument_lines();
    it_executes_two_explicit_zero_argument_calls_in_order();
    it_maps_delayed_execution_to_the_call_instead_of_its_last_argument();
    it_reads_a_local_before_its_call_result_assignment();
    it_copies_parameters_before_execution_and_results_after();
    it_accepts_named_parameters_and_results();
    it_rejects_mixed_argument_modes();
    it_rejects_parameters_after_results();
}

int main(void) {
    run();
    printf("unit/parser/calls: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
