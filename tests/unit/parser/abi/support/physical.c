/** @file
 * @brief Distinguish flat ABI records from Ama literals and enforce original physical whitespace.
 */
/** Pipe binds a C function rather than swallowing repeated separators/hash as raw Ama payload. */
static void it_keeps_abi_pipe_records_separate_from_ama_literal_rules(void) {
    const char *invalid[] = {
        "+  initialize\n- destroy\n", "+ initialize \n- destroy\n",
        "+ initialize\n- destroy\n ", "+ initialize\n- destroy\n  \n",
        "+ initialize\n- destroy\n|  literal\n", "+ initialize\n- destroy\n| literal  \n",
        "+ initialize\n- destroy\n|literal\n", "+ initialize\n- destroy\n| literal payload # not raw\n",
        "+ initialize\n- destroy\n| # no function\n", "+ initialize\n- destroy\n| \tdata\n",
        "+ initialize\n- destroy\n  | literal\n", "+ initialize\n- destroy\n| @!\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_test();
        ParseStatus status = parse_abi(invalid[index]);
        assert(status == PARSE_INVALID && !table.valid && diagnostic_valid(&table.error.diagnostic));
        destroy_abi();
    }
    setup_test();
    ParseStatus status = parse_abi("   # comments own their indentation  \r\n\r\n+ initialize   # trailing prefix\r\n- destroy#direct\r\n| literal  # payload is a C name\r\n");
    assert(status == PARSE_OK && table.valid);
    assert(layout_text_is(table.bindings[ABI_LITERAL].function, "literal"));
    assert(table.layout.first->ending == ENDING_CRLF);
    destroy_abi();
}
/** Every slot diagnoses duplicate marker sites independently from the function chosen for it. */
static void it_preserves_exact_duplicate_sites_for_every_protocol_slot(void) {
    static const char *const symbols[] = {"+", "-", "|", "=", "?", "[]"};
    for (size_t index = 0; index < ABI_BINDING_COUNT; index++) {
        char text[128];
        int length = snprintf(text, sizeof(text), "%s first\n%s second\n", symbols[index], symbols[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        setup_test();
        ParseStatus status = parse_abi(text);
        assert(status == PARSE_INVALID && table.error.diagnostic.related_count == 1);
        assert(table.error.diagnostic.related[0].location.span.start == 0);
        assert(table.error.diagnostic.primary.span.start == strlen(symbols[index]) + 7);
        assert(table.error.diagnostic.primary.span.end - table.error.diagnostic.primary.span.start == strlen(symbols[index]));
        destroy_abi();
    }
}
