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
    bool parsed = parse_text("add\n < 5\n < 7\n > result\n");
    assert(parsed);
    assert_stream_equals(output, "    instance_math_add.symbol_0 = 0;\n    instance_math_add.symbol_1 = 0;\n    instance_math_add.symbol_2 = 0;\n"
        "    instance_math_add.symbol_0 = 5;\n    instance_math_add.symbol_1 = 7;\n    module_math_add(&instance_math_add);\n    state->symbol_4 = instance_math_add.symbol_2;\n");
    destroy_test();
}

static void it_accepts_named_parameters_and_results(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add\n right < 7\n left < 5\n sum > result\n");
    assert(parsed && lookup_symbol(symbols, "main", "result") != NULL);
    destroy_test();
}

static void it_rejects_mixed_argument_modes(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add\n < 5\n right < 7\n");
    assert(!parsed && strstr(error.message, "cannot mix") != NULL);
    destroy_test();
}

static void it_rejects_parameters_after_results(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add\n > result\n < 5\n");
    assert(!parsed && strstr(error.message, "parameter after result") != NULL);
    destroy_test();
}

void run(void) {
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
