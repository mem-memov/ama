/**
 * @file
 * @brief Exercise empty and multichunk binary loads, every measured allocation failure and real I/O refusal.
 */

/** Publish an exact byte fixture and require complete file output before testing the loader. */
static void write_source_input(const unsigned char *bytes, size_t length) {
    FILE *stream = fopen("generated/bytes.input", "wb");
    assert(stream != NULL && fwrite(bytes, 1, length, stream) == length && fclose(stream) == 0);
}

/** Empty files have one EOF position, while a missing final LF never appends a line or source byte. */
static void it_loads_empty_and_unterminated_files_as_exact_original_bytes(void) {
    const unsigned char cases[][9] = {{0}, {'x'}, {'x','\r','\n'}, {0xe4,0xb8,0xad,'\r','\n',0xf0,0x9f,0x98,0x80}};
    const size_t lengths[] = {0,1,3,9};
    for (size_t selected = 0; selected < 4; selected++) {
        write_source_input(cases[selected], lengths[selected]);
        setup_test();
        const SourceText *source = NULL;
        assert(source_manager_load(&sources, "generated/bytes.input", &source) == SOURCE_OK);
        assert(source_length(source) == lengths[selected] && memcmp(source_bytes(source), cases[selected], lengths[selected]) == 0);
        SourcePosition actual, expected = linear_source_position(cases[selected], lengths[selected], lengths[selected]);
        assert(source_position(source, lengths[selected], &actual));
        assert(actual.line == expected.line && actual.column == expected.column && actual.line_end == expected.line_end);
        destroy_test();
    }
}

/** A large original stream crosses growth/CRLF/UTF-8 boundaries; failure cannot publish or alter an earlier source. */
static void it_unwinds_every_multichunk_load_allocation_preserving_existing_sources(void) {
    unsigned char bytes[7 * 4096 + 17];
    for (size_t index = 0; index < sizeof(bytes); index++) { bytes[index] = (unsigned char)(index % 251); }
    bytes[4095] = '\r';
    bytes[4096] = '\n';
    bytes[8191] = 0xe4;
    bytes[8192] = 0xb8;
    bytes[8193] = 0xad;
    write_source_input(bytes, sizeof(bytes));
    setup_test();
    const SourceText *source = NULL;
    assert(source_manager_load(&sources, "generated/bytes.input", &source) == SOURCE_OK);
    assert(source_length(source) == sizeof(bytes) && memcmp(source_bytes(source), bytes, sizeof(bytes)) == 0);
    size_t count = allocation_count;
    assert(count > 5);
    destroy_test();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_test();
        const SourceText *prior = NULL;
        assert(source_manager_add(&sources, "prior", (const unsigned char *)"old\r\n", 5, &prior) == SOURCE_OK);
        size_t baseline = live_allocations;
        source = prior;
        failure_at = allocation_count + failure;
        assert(source_manager_load(&sources, "generated/bytes.input", &source) == SOURCE_OUT_OF_MEMORY);
        assert(source == prior && sources.first == prior && live_allocations == baseline);
        assert(source_length(prior) == 5 && memcmp(source_bytes(prior), "old\r\n", 5) == 0);
        failure_at = 0;
        destroy_test();
    }
    setup_test();
    assert(source_manager_add(&sources, "retained", NULL, 0, &source) == SOURCE_OK);
    const SourceText *prior = source;
    assert(source_manager_load(&sources, "generated/no-such-input", &source) == SOURCE_IO_ERROR && source == prior);
    assert(source_manager_load(&sources, "generated", &source) == SOURCE_IO_ERROR && source == prior);
    assert(source_manager_load(&sources, NULL, &source) == SOURCE_INVALID_ARGUMENT && source == prior);
    assert(source_manager_load(&sources, "generated/bytes.input", NULL) == SOURCE_INVALID_ARGUMENT);
    assert(sources.first == prior);
    destroy_test();
}
