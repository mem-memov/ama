#include "../common.c"

static void it_combines_lexing_assignment_and_emission(void) {
    setup_test();
    bool parsed = parse_text("x\n42\ny\nx\n");
    assert(parsed && symbols->symbol_count == 2);
    assert_stream_equals(output, "    state->symbol_0 = 42;\n    state->symbol_1 = state->symbol_0;\n");
    destroy_test();
}

static void it_reports_an_unknown_source_without_terminating_the_process(void) {
    setup_test();
    bool parsed = parse_text("x\nunknown\n");
    assert(!parsed && error.error_code == ERROR_INVALID && error.line_number == 2);
    assert(strcmp(error.module, "main") == 0);
    destroy_test();
}

static void it_combines_nested_loops_branches_and_return(void) {
    setup_test();
    bool parsed = parse_text("x\n1\nx\n{\n x\n ?\n  >\n !\n}\n");
    assert(parsed);
    assert_stream_equals(output, "    state->symbol_0 = 1;\n    while (state->symbol_0) {\n    if (state->symbol_0) {\n    return;\n    }\n    break;\n    }\n");
    destroy_test();
}

static void it_rejects_a_blank_between_a_condition_and_its_marker(void) {
    setup_test();
    bool parsed = parse_text("x\n1\nx\n\n?\n y\n 2\n");
    assert(!parsed && strstr(error.message, "blank line between") != NULL);
    destroy_test();
}

void run(void) {
    it_combines_lexing_assignment_and_emission();
    it_reports_an_unknown_source_without_terminating_the_process();
    it_combines_nested_loops_branches_and_return();
    it_rejects_a_blank_between_a_condition_and_its_marker();
}

int main(void) {
    run();
    printf("unit/parser/statements: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
