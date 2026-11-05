#include "../common.c"

static void it_emits_values_and_control_flow_to_the_given_output(void) {
    setup_test();
    emit_assign_const(&emitter, 1, 42);
    emit_copy(&emitter, 2, 1);
    emit_branch_start(&emitter, 2);
    emit_return(&emitter);
    emit_branch_else(&emitter);
    emit_break(&emitter);
    emit_branch_end(&emitter);
    assert_stream_equals(output, "  v_1 = 42;\n  v_2 = v_1;\n  if (v_2) {\n"
                                 "  return;\n  } else {\n  break;\n  }\n");
    destroy_test();
}

static void it_initializes_only_the_selected_modules_variables(void) {
    setup_test();
    Token value = {.type = TOK_LITERAL, .lexeme = "9"};
    Symbol* result = add_declarative_symbol(symbols, "callee", "answer", SYM_RETURN, value, &error);
    Symbol* local = add_local_variable(symbols, "callee", "scratch", &error);
    Symbol* other = add_local_variable(symbols, "other", "scratch", &error);
    assert(result && local && other);
    emit_call_init(&emitter, "callee");
    assert_stream_equals(output, "  v_0 = 9;\n  v_1 = 0;\n");
    destroy_test();
}

static void it_keeps_two_emitters_independent(void) {
    setup_test();
    FILE* other = tmpfile();
    assert(other != NULL);
    Emitter second;
    emitter_init(&second, other, symbols);
    emit_assign_const(&emitter, 1, 5);
    emit_assign_const(&second, 1, 7);
    assert_stream_equals(output, "  v_1 = 5;\n");
    assert_stream_equals(other, "  v_1 = 7;\n");
    close_file(other);
    destroy_test();
}

void run(void) {
    it_emits_values_and_control_flow_to_the_given_output();
    it_initializes_only_the_selected_modules_variables();
    it_keeps_two_emitters_independent();
}

int main(void) {
    run();
    printf("unit/codegen/emitter: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
