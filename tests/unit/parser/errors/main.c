#include "../ast-common.c"

static void it_reports_specific_expected_constructs_at_exact_source_offsets(void) {
    const struct {
        const char *text;
        ParserExpectation expected;
        size_t offset;
    } cases[] = {
        {"@!\nx\n.type&", EXPECT_TYPE_LINE, 5},
        {"@!\nx\n |12", EXPECT_COPY_SOURCE, 6},
        {"@!\nx\n items[position", EXPECT_RIGHT_BRACKET, 20},
        {"@!\nworker", EXPECT_STATEMENT_TAIL, 3},
        {"@!\nready\n\n?", EXPECT_CONDITION_ADJACENCY, 3},
        {"@!\nworker!\n >> answer", EXPECT_ENTRY_BINDINGS, 12}
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

static void it_provides_exact_help_and_never_uses_names_to_reclassify_syntax(void) {
    const char *programs[] = {"@!\nknown\n.type\ndestination\n known", "@!\nother\n.type\ndestination\n known"};
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

static void it_keeps_layout_and_utf8_failures_in_the_same_structured_error_contract(void) {
    const char *invalid[] = {"@!\n\t", "draft\xff"};
    const ParserExpectation expected[] = {EXPECT_LAYOUT, EXPECT_UTF8};
    for (size_t index = 0; index < 2; index++) {
        setup_tree();
        ParseStatus parsed = parse_text(invalid[index]);
        assert(parsed == PARSE_INVALID && module.error.expected == expected[index]);
        assert(strcmp(module.error.diagnostic.expected, parser_expected_text(expected[index])) == 0);
        destroy_tree();
    }
}

static void it_produces_valid_diagnostics_for_every_truncated_byte_prefix(void) {
    const char *program = "@=\n常量\n.type\n |@! # data\n@!\nready\n{\n worker!\n  <\n  > answer\n %\n}\n";
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

void run(void) {
    it_reports_specific_expected_constructs_at_exact_source_offsets();
    it_provides_exact_help_and_never_uses_names_to_reclassify_syntax();
    it_keeps_layout_and_utf8_failures_in_the_same_structured_error_contract();
    it_produces_valid_diagnostics_for_every_truncated_byte_prefix();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
