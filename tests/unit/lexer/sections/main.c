#include "../common.c"

static void it_finds_sections_and_allows_blank_lines_in_code(void) {
    setup_test();
    source = text_file("x\n0\n\nresult\n1\n\nresult\n42\n\nresult\n7\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_line == 1 && sections.results_line == 4);
    assert(sections.code_line == 7 && sections.code_start == sections.results_end + 1);
    destroy_test();
}

static void it_accepts_three_empty_sections(void) {
    setup_test();
    source = text_file("\n\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(split && sections.parameters_start == sections.parameters_end);
    assert(sections.code_start == sections.code_end);
    destroy_test();
}

static void it_rejects_missing_separators(void) {
    setup_test();
    source = text_file("x\n0\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.error_code == ERROR_INVALID);
    destroy_test();
}

static void it_rejects_tabs_with_the_physical_line_number(void) {
    setup_test();
    source = text_file("\n\nx\n\t42\n");
    ModuleSections sections;
    bool split = split_module_sections(source, &sections, &error);
    assert(!split && error.error_code == ERROR_INVALID && error.line_number == 4);
    destroy_test();
}

void run(void) {
    it_finds_sections_and_allows_blank_lines_in_code();
    it_accepts_three_empty_sections();
    it_rejects_missing_separators();
    it_rejects_tabs_with_the_physical_line_number();
}

int main(void) {
    run();
    printf("unit/lexer/sections: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
