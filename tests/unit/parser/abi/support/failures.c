/** @file
 * @brief Check every truncated source and every composed source/line allocation failure.
 */
/** Prefix acceptance may change, but every outcome has bounded original-byte locations and complete lifecycle slots on success. */
static void it_checks_every_abi_truncation_without_publishing_partial_success(void) {
    const char *text = "# physical comment\r\n+ 初始化\r\n- destroy\n| literal # comment\n= clone\n? condition\n[] to_index";
    size_t length = strlen(text);
    char *prefix = malloc(length + 1);
    assert(prefix != NULL);
    for (size_t end = 0; end <= length; end++) {
        memcpy(prefix, text, end);
        prefix[end] = 0;
        setup_test();
        ParseStatus status = parse_abi(prefix);
        if (status == PARSE_OK) { assert(table.valid && table.bindings[ABI_DEFAULT].present && table.bindings[ABI_DESTROY].present); }
        else { assert(!table.valid && diagnostic_valid(&table.error.diagnostic)); }
        destroy_abi();
    }
    free(prefix);
}
/** Every failed allocation, including source admission before ABI parsing, has one safely destructible owner. */
static void it_unwinds_every_source_and_abi_allocation_failure(void) {
    const char *text = "# comment\n+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] to_index\n";
    setup_test();
    ParseStatus status = parse_abi(text);
    assert(status == PARSE_OK);
    size_t total = allocation_count;
    destroy_abi();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_test();
        failure_at = failure;
        abi_table_init(&table, sources.allocator);
        const SourceText *source = NULL;
        SourceStatus added = source_manager_add(&sources, "native.abi", (const unsigned char *)text, strlen(text), &source);
        if (added == SOURCE_OK) {
            status = parser_abi_parse(&table, source);
            assert(status == PARSE_OUT_OF_MEMORY && !table.valid);
            assert(table.error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        } else { assert(added == SOURCE_OUT_OF_MEMORY); }
        destroy_abi();
    }
}
