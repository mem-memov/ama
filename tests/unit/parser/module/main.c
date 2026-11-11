#include "../common.c"

static void it_keeps_unfinished_files_drafts_without_literal_or_comment_activation(void) {
    const char *drafts[] = {"", "@<\nunfinished\t", "# @!\n |@C\n", " @!\n", "@unknown"};
    for (size_t index = 0; index < 5; index++) {
        setup_test();
        LayoutStatus read = read_layout(drafts[index]);
        assert(read == LAYOUT_OK);
        ModuleSections sections;
        Diagnostic error;
        bool parsed = parser_sections(&layout, &sections, &error);
        assert(parsed && sections.kind == MODULE_DRAFT);
        destroy_layout();
    }
}

static void it_accepts_empty_constants_and_both_implementation_kinds(void) {
    const char *sources[] = {"@=", "@! # empty", "@=\n@<\n@>\n@C\n# native", "@<\n@>\n@!\n"};
    const ParsedModuleKind kinds[] = {MODULE_CONSTANTS, MODULE_AMA, MODULE_NATIVE, MODULE_AMA};
    for (size_t index = 0; index < 4; index++) {
        setup_test();
        LayoutStatus read = read_layout(sources[index]);
        assert(read == LAYOUT_OK);
        ModuleSections sections;
        Diagnostic error;
        bool parsed = parser_sections(&layout, &sections, &error);
        assert(parsed && sections.kind == kinds[index]);
        destroy_layout();
    }
}

static void it_rejects_active_section_order_duplicates_and_native_content(void) {
    const char *invalid[] = {"@=\n@<", "@=\n@>\n", "@!\n@!", "@>\n@<\n@!",
        "@!\n@C", "@!\n@other", "@C\nvalue", "unfinished\n@!", "@!\n @<"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_test();
        LayoutStatus read = read_layout(invalid[index]);
        assert(read == LAYOUT_OK);
        ModuleSections sections = {0};
        Diagnostic error;
        bool parsed = parser_sections(&layout, &sections, &error);
        assert(!parsed && diagnostic_valid(&error) && sections.kind == MODULE_DRAFT);
        destroy_layout();
    }
}

void run(void) {
    it_keeps_unfinished_files_drafts_without_literal_or_comment_activation();
    it_accepts_empty_constants_and_both_implementation_kinds();
    it_rejects_active_section_order_duplicates_and_native_content();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
