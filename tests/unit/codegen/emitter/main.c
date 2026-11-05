#include "../common.c"

static void it_emits_values_and_control_flow_to_the_given_output(void) {
    setup_test();
    emit_constant_assignment(&emitter, 1, 42);
    emit_copy(&emitter, 2, 1);
    emit_branch_start(&emitter, 2);
    emit_return(&emitter);
    emit_branch_else(&emitter);
    emit_break(&emitter);
    emit_branch_end(&emitter);
    assert_stream_equals(output, "    symbol_1 = 42;\n    symbol_2 = symbol_1;\n    if (symbol_2) {\n"
        "    return;\n    } else {\n    break;\n    }\n");
    destroy_test();
}

static void it_initializes_only_the_selected_modules_variables(void) {
    setup_test();
    Token value = {.type = TOKEN_LITERAL, .lexeme = "9"};
    Symbol* result_symbol = add_declarative_symbol(symbols, "callee", "answer", SYMBOL_RESULT, value, &error);
    Symbol* local_symbol = add_local_variable(symbols, "callee", "scratch", &error);
    Symbol* unrelated_symbol = add_local_variable(symbols, "other", "scratch", &error);
    assert(result_symbol && local_symbol && unrelated_symbol);
    emit_call_init(&emitter, "callee");
    assert_stream_equals(output, "    symbol_0 = 9;\n    symbol_1 = 0;\n");
    destroy_test();
}

static void it_keeps_two_emitters_independent(void) {
    setup_test();
    FILE* second_output = tmpfile();
    assert(second_output != NULL);
    Emitter second_emitter;
    emitter_init(&second_emitter, second_output, symbols);
    emit_constant_assignment(&emitter, 1, 5);
    emit_constant_assignment(&second_emitter, 1, 7);
    assert_stream_equals(output, "    symbol_1 = 5;\n");
    assert_stream_equals(second_output, "    symbol_1 = 7;\n");
    close_file(second_output);
    destroy_test();
}

void run(void) {
    it_emits_values_and_control_flow_to_the_given_output();
    it_initializes_only_the_selected_modules_variables();
    it_keeps_two_emitters_independent();
}

int main(void) {
    run();
    printf("unit/codegen/emitter: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
