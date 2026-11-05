#include "../common.c"

static void declare_add(void) {
    Token zero = {.type = TOK_LITERAL, .lexeme = "0"};
    Token path = {.type = TOK_PATH, .lexeme = ".math.add"};
    Symbol* left = add_declarative_symbol(symbols, "math/add", "left", SYM_PARAM, zero, &error);
    Symbol* right = add_declarative_symbol(symbols, "math/add", "right", SYM_PARAM, zero, &error);
    Symbol* sum = add_declarative_symbol(symbols, "math/add", "sum", SYM_RETURN, zero, &error);
    Symbol* alias = add_declarative_symbol(symbols, "main", "add", SYM_ALIAS, path, &error);
    int node = graph_add_node(graph, "math/add", &error);
    assert(left && right && sum && alias && node >= 0);
}

static void it_copies_parameters_before_execution_and_results_after(void) {
    setup_test();
    declare_add();
    bool parsed = parse_text("add\n < 5\n < 7\n > result\n");
    assert(parsed);
    assert_stream_equals(output, "  v_0 = 0;\n  v_1 = 0;\n  v_2 = 0;\n"
                                 "  v_0 = 5;\n  v_1 = 7;\n  mod_math_add();\n  v_4 = v_2;\n");
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
    printf("unit/parser/calls: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
