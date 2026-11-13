/**
 * @file
 * @brief Check section activation, ordering and literal-aware structural boundaries.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component keeps unfinished files drafts without literal or comment activation.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_keeps_unfinished_files_drafts_without_literal_or_comment_activation(void) {
    const char *drafts[] = {"", "@<\nunfinished", "# @!\n |@C\n", " @!\n", "@unknown"};
    for (size_t index = 0; index < 5; index++) {
        setup_test();
        LayoutStatus read = read_layout(drafts[index]);
        assert(read == LAYOUT_OK);
        ModuleSections sections;
        ParseError error;
        bool parsed = parser_sections(&layout, &sections, &error);
        assert(parsed && sections.kind == MODULE_DRAFT);
        destroy_layout();
    }
}

/**
 * Assert that the component accepts empty constants and both implementation kinds. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_accepts_empty_constants_and_both_implementation_kinds(void) {
    const char *sources[] = {"@=", "@! # empty", "@=\n@<\n@>\n@C\n# native", "@<\n@>\n@!\n"};
    const ParsedModuleKind kinds[] = {MODULE_CONSTANTS, MODULE_AMA, MODULE_NATIVE, MODULE_AMA};
    for (size_t index = 0; index < 4; index++) {
        setup_test();
        LayoutStatus read = read_layout(sources[index]);
        assert(read == LAYOUT_OK);
        ModuleSections sections;
        ParseError error;
        bool parsed = parser_sections(&layout, &sections, &error);
        assert(parsed && sections.kind == kinds[index]);
        destroy_layout();
    }
}

/**
 * Assert that the component rejects active section order duplicates and native content. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_rejects_active_section_order_duplicates_and_native_content(void) {
    const char *invalid[] = {"@=\n@<", "@=\n@>\n", "@!\n@!", "@>\n@<\n@!",
        "@!\n@C", "@!\n@other", "@C\nvalue", "unfinished\n@!", "@!\n @<"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_test();
        LayoutStatus read = read_layout(invalid[index]);
        assert(read == LAYOUT_OK);
        ModuleSections sections = {0};
        ParseError error;
        bool parsed = parser_sections(&layout, &sections, &error);
        assert(!parsed && diagnostic_valid(&error.diagnostic) && sections.kind == MODULE_DRAFT);
        destroy_layout();
    }
}

/** Preserve actual lexical expectations and all context while assigning a broad parser key. */
static void it_preserves_precise_layout_expectations_during_parser_adaptation(void) {
    const char *inputs[] = {"@!\n\titem\n", "@!\nitem\rvalue\n", "@!\n\xff"};
    const char *expected[] = {"ASCII spaces", "LF or CRLF", "well-formed UTF-8"};
    for (size_t index = 0; index < 3; index++) {
        setup_test();
        LayoutStatus status = read_layout(inputs[index]);
        assert(status == LAYOUT_INVALID);
        Diagnostic original = layout.diagnostic;
        DiagnosticRelated note = {.location = original.primary, .role = "original check"};
        original.related = &note;
        original.related_count = 1;
        original.cause = "exact originating cause";
        original.help = "explicit originating help";
        ParseError error;
        parser_error_from_layout(&error, &original);
        assert(diagnostic_valid(&error.diagnostic));
        assert(strcmp(error.diagnostic.expected, expected[index]) == 0);
        assert(error.diagnostic.expected == original.expected);
        assert(error.diagnostic.message == original.message);
        assert(error.diagnostic.cause == original.cause);
        assert(error.diagnostic.help == original.help);
        assert(error.diagnostic.related == &note && error.diagnostic.related_count == 1);
        assert(error.diagnostic.primary.span.source == original.primary.span.source);
        assert(error.diagnostic.primary.span.start == original.primary.span.start);
        assert(error.diagnostic.primary.span.end == original.primary.span.end);
        assert(error.expected == (index == 2 ? EXPECT_UTF8 : EXPECT_LAYOUT));
        destroy_layout();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_preserves_precise_layout_expectations_during_parser_adaptation();
    it_keeps_unfinished_files_drafts_without_literal_or_comment_activation();
    it_accepts_empty_constants_and_both_implementation_kinds();
    it_rejects_active_section_order_duplicates_and_native_content();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
