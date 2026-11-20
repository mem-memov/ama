/**
 * @file
 * @brief Check precise structural parser diagnostics and partial allocation cleanup.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../ast-common.c"

/**
 * Assert that the component reports specific expected constructs at exact source offsets. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reports_specific_expected_constructs_at_exact_source_offsets(void) {
    const struct {
        const char *text;
        ParserExpectation expected;
        size_t offset;
    } cases[] = {
        {"@!\nx\n/test/fixture/v1.0.0/src/type&", EXPECT_TYPE_LINE, 5},
        {"@!\nx\n |12", EXPECT_COPY_SOURCE, 6},
        {"@!\nx\n items[position", EXPECT_RIGHT_BRACKET, 20},
        {"@!\nworker", EXPECT_STATEMENT_TAIL, 3},
        {"@!\nready\n\n?", EXPECT_CONDITION_ADJACENCY, 3},
        {"@!\nworker !\n >> answer", EXPECT_ENTRY_BINDINGS, 13}
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        setup_tree();
        ParseStatus parsed = parse_text(cases[index].text);
        assert(parsed == PARSE_INVALID && module.error.expected == cases[index].expected);
        assert(module.error.diagnostic.primary.span.start == cases[index].offset);
        assert(strcmp(module.error.diagnostic.expected, parser_expected_text(cases[index].expected)) == 0);
        destroy_tree();
    }
}

/**
 * Assert that the component provides exact help and never uses names to reclassify syntax. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_provides_exact_help_and_never_uses_names_to_reclassify_syntax(void) {
    const char *programs[] = {"@!\nknown\n/test/fixture/v1.0.0/src/type\ndestination\n known", "@!\nother\n/test/fixture/v1.0.0/src/type\ndestination\n known"};
    for (size_t index = 0; index < 2; index++) {
        setup_tree();
        ParseStatus parsed = parse_text(programs[index]);
        assert(parsed == PARSE_OK && module.body.last->kind == AST_ASSIGNMENT);
        destroy_tree();
    }
    setup_tree();
    ParseStatus parsed = parse_text("@!\ndestination\n |literal");
    assert(parsed == PARSE_INVALID && module.error.diagnostic.help != NULL);
    assert(strstr(module.error.diagnostic.help, "typed declaration") != NULL);
    destroy_tree();
}

/**
 * Assert that the component keeps layout and utf8 failures in the same structured error
 * contract. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_keeps_layout_and_utf8_failures_in_the_same_structured_error_contract(void) {
    const char *invalid[] = {"@!\n\t", "draft\xff"};
    const ParserExpectation expected[] = {EXPECT_LAYOUT, EXPECT_UTF8};
    const char *precise[] = {"ASCII spaces", "well-formed UTF-8"};
    for (size_t index = 0; index < 2; index++) {
        setup_tree();
        ParseStatus parsed = parse_text(invalid[index]);
        assert(parsed == PARSE_INVALID && module.error.expected == expected[index]);
        assert(strcmp(module.error.diagnostic.expected, precise[index]) == 0);
        destroy_tree();
    }
}

/**
 * Assert that the component produces valid diagnostics for every truncated byte prefix. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_produces_valid_diagnostics_for_every_truncated_byte_prefix(void) {
    const char *program = "@=\n常量\n/test/fixture/v1.0.0/src/type\n |@! # data\n@!\nready\n{\n worker !\n  <\n  > answer\n %\n}\n";
    size_t length = strlen(program);
    char *prefix = malloc(length + 1);
    assert(prefix != NULL);
    for (size_t end = 0; end <= length; end++) {
        memcpy(prefix, program, end);
        prefix[end] = 0;
        setup_tree();
        ParseStatus parsed = parse_text(prefix);
        assert(parsed == PARSE_OK ? module.valid : diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
    free(prefix);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_reports_specific_expected_constructs_at_exact_source_offsets();
    it_provides_exact_help_and_never_uses_names_to_reclassify_syntax();
    it_keeps_layout_and_utf8_failures_in_the_same_structured_error_contract();
    it_produces_valid_diagnostics_for_every_truncated_byte_prefix();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
