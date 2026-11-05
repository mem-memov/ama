#include "../common.c"

static void it_finds_explicit_sections_and_preserves_physical_lines(void) {
    setup_test();
    source = text_file("@<\nvalue\n0\n\n@>\nresult\n1\n\n@Ama\nresult\n42\n\nresult\n7\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_line == 2 && sections.results_line == 6);
    assert(sections.code_line == 10 && sections.parameters_end < sections.results_start);
    bool initialized = init_lexer(&lexer, source, sections.results_start, sections.results_end,
        sections.results_line, &error);
    assert(initialized);
    Token result_name = get_next_token(&lexer);
    assert(result_name.type == TOKEN_IDENTIFIER && result_name.line_number == 6);
    assert(strcmp(result_name.lexeme, "result") == 0);
    destroy_test();
}

static void it_accepts_a_code_only_module(void) {
    setup_test();
    source = text_file("@Ama\nvalue\n42\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_start == sections.parameters_end);
    assert(sections.results_start == sections.results_end && sections.code_line == 2);
    destroy_test();
}

static void it_accepts_results_without_parameters(void) {
    setup_test();
    source = text_file("@>\nresult\n9\n@Ama\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_start == sections.parameters_end);
    assert(sections.results_line == 2 && sections.code_line == 5);
    destroy_test();
}

static void it_accepts_parameters_without_results(void) {
    setup_test();
    source = text_file("@<\nvalue\n0\n@Ama\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_line == 2);
    assert(sections.results_start == sections.results_end && sections.code_line == 5);
    destroy_test();
}

static void it_accepts_explicit_empty_sections_without_a_final_newline(void) {
    setup_test();
    source = text_file("@<\n@>\n@Ama");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_start == sections.parameters_end);
    assert(sections.results_start == sections.results_end);
    assert(sections.code_start == sections.code_end);
    destroy_test();
}

static void it_allows_comments_blank_lines_and_crlf_around_annotations(void) {
    setup_test();
    source = text_file("# @C is only a comment\r\n\r\n@< # parameters\r\n\r\n@>   \r\n@Ama # implementation\r\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_line == 4 && sections.results_line == 6);
    assert(sections.code_line == 7);
    destroy_test();
}

static void it_rejects_duplicate_annotations(void) {
    setup_test();
    source = text_file("@<\n@<\n@Ama\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.line_number == 2);
    assert(strstr(error.message, "duplicate or out-of-order") != NULL);
    destroy_test();
}

static void it_rejects_out_of_order_annotations(void) {
    setup_test();
    source = text_file("@>\n@<\n@Ama\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.line_number == 2);
    assert(strstr(error.message, "duplicate or out-of-order") != NULL);
    destroy_test();
}

static void it_accepts_native_annotations_without_an_ama_body(void) {
    setup_test();
    source = text_file("@C\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.implementation == MODULE_C_IMPLEMENTATION);
    assert(sections.code_start == sections.code_end);
    destroy_test();
}

static void it_rejects_indented_annotations(void) {
    setup_test();
    source = text_file(" @Ama\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && strstr(error.message, "indentation 0") != NULL);
    destroy_test();
}

static void it_rejects_the_legacy_blank_line_format(void) {
    setup_test();
    source = text_file("value\n0\n\nresult\n0\n\nresult\nvalue\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.line_number == 1);
    assert(strstr(error.message, "content before a section annotation") != NULL);
    destroy_test();
}

static void it_classifies_unfinished_interfaces_without_parsing_them(void) {
    setup_test();
    source = text_file("@<\nvalue\n0\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.implementation == MODULE_DECLARATION_ONLY);
    destroy_test();
}

static void it_rejects_tabs_with_the_physical_line_number(void) {
    setup_test();
    source = text_file("@Ama\nvalue\n\t42\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.error_code == ERROR_INVALID && error.line_number == 3);
    destroy_test();
}

static void it_rejects_a_stray_carriage_return_on_an_annotation(void) {
    setup_test();
    source = text_file("@Ama\r");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.error_code == ERROR_INVALID && error.line_number == 1);
    destroy_test();
}

static void it_rejects_ama_code_after_a_native_annotation(void) {
    setup_test();
    source = text_file("@C\nvalue\n42\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.line_number == 2);
    assert(strstr(error.message, "must not contain an Ama code body") != NULL);
    destroy_test();
}

static void it_rejects_two_implementation_selectors(void) {
    setup_test();
    source = text_file("@Ama\n@C\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && strstr(error.message, "duplicate or out-of-order") != NULL);
    destroy_test();
}

void run(void) {
    it_rejects_ama_code_after_a_native_annotation();
    it_rejects_two_implementation_selectors();
    it_finds_explicit_sections_and_preserves_physical_lines();
    it_accepts_a_code_only_module();
    it_accepts_results_without_parameters();
    it_accepts_parameters_without_results();
    it_accepts_explicit_empty_sections_without_a_final_newline();
    it_allows_comments_blank_lines_and_crlf_around_annotations();
    it_rejects_duplicate_annotations();
    it_rejects_out_of_order_annotations();
    it_accepts_native_annotations_without_an_ama_body();
    it_rejects_indented_annotations();
    it_rejects_the_legacy_blank_line_format();
    it_classifies_unfinished_interfaces_without_parsing_them();
    it_rejects_tabs_with_the_physical_line_number();
    it_rejects_a_stray_carriage_return_on_an_annotation();
}

int main(void) {
    run();
    printf("unit/lexer/sections: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
