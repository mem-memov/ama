#include "../../source/common.c"
#include "parser/abi/abi.h"

static AbiTable table;

static ParseStatus parse_abi(const char *text) {
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "native.abi", (const unsigned char *)text, strlen(text), &source);
    assert(added == SOURCE_OK);
    abi_table_init(&table, sources.allocator);
    return parser_abi_parse(&table, source);
}

static void destroy_abi(void) {
    abi_table_destroy(&table);
    destroy_test();
}

static void it_reads_all_six_symbols_in_any_order_with_physical_spans(void) {
    setup_test();
    ParseStatus parsed = parse_abi(" # comment\r\n[] to_index\r\n? to_condition # optional\n= clone\n"
        "| from_literal\n- destroy\n+ initialize");
    assert(parsed == PARSE_OK && table.valid);
    for (size_t index = 0; index < ABI_BINDING_COUNT; index++) {
        assert(table.bindings[index].present && source_span_valid(table.bindings[index].function));
    }
    assert(layout_text_is(table.bindings[ABI_INDEX].function, "to_index"));
    assert(layout_text_is(table.bindings[ABI_DEFAULT].function, "initialize"));
    destroy_abi();
}

static void it_accepts_international_native_names_and_c_universal_character_spellings(void) {
    const char *names[] = {"初始化", "native_2", "_initialize", "\\u521d\\u59cb\\u5316", "$initialize"};
    for (size_t index = 0; index < 5; index++) {
        setup_test();
        char text[256] = "+ ";
        strcat(text, names[index]);
        strcat(text, "\n- destroy\n");
        ParseStatus parsed = parse_abi(text);
        assert(parsed == PARSE_OK && layout_text_is(table.bindings[ABI_DEFAULT].function, names[index]));
        destroy_abi();
    }
}

static void it_rejects_malformed_unknown_indented_and_duplicate_records(void) {
    const char *invalid[] = {"@C", "++ initialize", "+initialize", "+ init extra", "+ 2bad",
        "+ init()", "+ return", "+ \\u0001", "+ e-foo", "[ ] index", " + init", "| literal\tvalue",
        "# comment\t", "+ init\r- destroy", "+ init\n- destroy\n+ again", "+ init\n+ again\n- destroy", "+ bad\xff"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_test();
        ParseStatus parsed = parse_abi(invalid[index]);
        assert(parsed == PARSE_INVALID && !table.valid && diagnostic_valid(&table.error.diagnostic));
        if (index == 14 || index == 15) {
            assert(table.error.diagnostic.related_count == 1);
            assert(table.error.diagnostic.related[0].location.span.start == 0);
        }
        destroy_abi();
    }
}

static void it_rejects_invalid_ucn_and_native_identifier_boundary_cases(void) {
    const char *invalid[] = {"+ \\uD800", "+ \\U00110000", "+ \\u0061", "+ \\u0300name",
        "+ \\u123", "+ \\U0000FFFF", "+ \\u00A0name", "+ # missing function"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_test();
        ParseStatus parsed = parse_abi(invalid[index]);
        assert(parsed == PARSE_INVALID && table.error.expected == EXPECT_C_IDENTIFIER);
        destroy_abi();
    }
}

static void it_points_missing_mandatory_bindings_at_eof(void) {
    const char *inputs[] = {"", "+ initialize", "- destroy", "? condition\n[] index\n"};
    for (size_t index = 0; index < 4; index++) {
        setup_test();
        ParseStatus parsed = parse_abi(inputs[index]);
        assert(parsed == PARSE_INVALID);
        assert(table.error.diagnostic.primary.span.start == strlen(inputs[index]));
        assert(table.error.diagnostic.primary.span.end == strlen(inputs[index]));
        assert(table.error.diagnostic.expected != NULL);
        destroy_abi();
    }
}

static void it_cleans_every_failed_abi_line_allocation(void) {
    const char *text = "+ initialize\n- destroy\n? condition\n";
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_test();
        const SourceText *source;
        SourceStatus added = source_manager_add(&sources, "native.abi", (const unsigned char *)text, strlen(text), &source);
        assert(added == SOURCE_OK);
        abi_table_init(&table, sources.allocator);
        allocation_count = 0;
        failure_at = failure;
        ParseStatus parsed = parser_abi_parse(&table, source);
        assert(parsed == PARSE_OUT_OF_MEMORY && !table.valid);
        destroy_abi();
    }
}

void run(void) {
    it_reads_all_six_symbols_in_any_order_with_physical_spans();
    it_accepts_international_native_names_and_c_universal_character_spellings();
    it_rejects_malformed_unknown_indented_and_duplicate_records();
    it_rejects_invalid_ucn_and_native_identifier_boundary_cases();
    it_points_missing_mandatory_bindings_at_eof();
    it_cleans_every_failed_abi_line_allocation();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
