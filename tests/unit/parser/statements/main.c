#include "../common.c"

static void it_combines_lexing_assignment_and_emission(void) {
    setup_test();
    bool parsed = parse_text("x\n42\ny\nx\n");
    assert(parsed && symbols->symbol_count == 2);
    assert_stream_equals(output, "    state->ama_field_134c0b29bc609d67 = 42;\n    state->ama_field_134c0a29bc609bb4 = state->ama_field_134c0b29bc609d67;\n");
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
    assert_stream_equals(output, "    state->ama_field_134c0b29bc609d67 = 1;\n    while (state->ama_field_134c0b29bc609d67) {\n    if (state->ama_field_134c0b29bc609d67) {\n    return;\n    }\n    break;\n    }\n");
    destroy_test();
}

static void it_rejects_a_blank_between_a_condition_and_its_marker(void) {
    setup_test();
    bool parsed = parse_text("x\n1\nx\n\n?\n y\n 2\n");
    assert(!parsed && strstr(error.message, "blank line between") != NULL);
    destroy_test();
}

static void it_discovers_persistent_targets_before_reading_them(void) {
    setup_test();
    bool parsed = parse_text("snapshot\nsaved\nflag\n1\nflag\n?\n saved\n 8\n");
    assert(parsed);
    Symbol* saved = lookup_symbol(symbols, "main", "saved");
    Symbol* snapshot = lookup_symbol(symbols, "main", "snapshot");
    assert(saved && snapshot && saved->kind == SYMBOL_LOCAL_VARIABLE);
    assert(symbols->symbol_count == 3);
    destroy_test();
}

static void it_maps_control_flow_and_restores_generated_scaffolding(void) {
    setup_test();
    emitter.source_mapping_enabled = true;
    bool parsed = parse_text_at("x\n1\nx\n{\n x\n ?\n  >\n :\n  !\n}\n", "flow.ama");
    assert(parsed);
    emit_module_end(&emitter);
    rewind(output);
    char text[8192];
    size_t length = fread(text, 1, sizeof(text) - 1, output);
    assert(!ferror(output));
    text[length] = '\0';
    assert(strstr(text, "#line 7 \"flow.ama\"\n    return;") != NULL);
    assert(strstr(text, "#line 8 \"flow.ama\"\n    } else {") != NULL);
    assert(strstr(text, "#line 9 \"flow.ama\"\n    break;") != NULL);
    assert(strstr(text, "#line 10 \"flow.ama\"\n    }") != NULL);
    assert(strstr(text, "#line 1 \"<ama-generated>\"\n}\n") != NULL);
    Symbol* value = lookup_symbol(symbols, "main", "x");
    assert(value && value->declaration.line == 1 && value->declaration.column == 1);
    destroy_test();
}

void run(void) {
    it_maps_control_flow_and_restores_generated_scaffolding();
    it_discovers_persistent_targets_before_reading_them();
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
