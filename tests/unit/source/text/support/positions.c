/**
 * @file
 * @brief Validate physical source coordinates and EOF against original bytes, including all small CR/LF combinations.
 */

/** Compute one coordinate directly from the physical-byte definition, independently of stored line tables. */
static SourcePosition linear_source_position(const unsigned char *bytes, size_t length, size_t offset) {
    size_t line = 1, start = 0;
    for (size_t index = 0; index < offset; index++) {
        if (bytes[index] == '\n') { line++; start = index + 1; }
    }
    size_t end = start;
    while (end < length && bytes[end] != '\n') { end++; }
    if (end < length && end > start && bytes[end - 1] == '\r') { end--; }
    return (SourcePosition){line, offset - start + 1, start, end};
}

/** Observe every offset, including EOF, and the unchanged failure output through the public source API. */
static void expect_original_positions(const unsigned char *bytes, size_t length) {
    setup_test();
    const SourceText *source = NULL;
    assert(source_manager_add(&sources, "original", bytes, length, &source) == SOURCE_OK);
    assert(source_length(source) == length && memcmp(source_bytes(source), bytes, length) == 0);
    for (size_t offset = 0; offset <= length; offset++) {
        SourcePosition actual, expected = linear_source_position(bytes, length, offset);
        assert(source_position(source, offset, &actual));
        assert(actual.line == expected.line && actual.column == expected.column);
        assert(actual.line_start == expected.line_start && actual.line_end == expected.line_end);
        assert(source_span_valid((SourceSpan){source, offset, offset}));
    }
    SourcePosition unchanged = {91,92,93,94};
    assert(!source_position(source, length + 1, &unchanged));
    assert(!source_position(NULL, 0, &unchanged) && !source_position(source, 0, NULL));
    assert(unchanged.line == 91 && unchanged.column == 92 && unchanged.line_start == 93 && unchanged.line_end == 94);
    assert(!source_span_valid((SourceSpan){source, 0, length + 1}) && !source_span_valid((SourceSpan){0}));
    destroy_test();
}

/** All byte sequences through length six over text, CR, LF, NUL and two UTF-8 bytes retain exact physical positions. */
static void it_preserves_eof_and_every_small_physical_line_boundary_without_fabricated_bytes(void) {
    static const unsigned char alphabet[] = {'a','\r','\n',0,0xc3,0xa9};
    unsigned char bytes[6];
    size_t possibilities = 1;
    for (size_t length = 0; length <= sizeof(bytes); length++) {
        for (size_t selected = 0; selected < possibilities; selected++) {
            size_t value = selected;
            for (size_t index = 0; index < length; index++) {
                bytes[index] = alphabet[value % sizeof(alphabet)];
                value /= sizeof(alphabet);
            }
            expect_original_positions(bytes, length);
        }
        possibilities *= sizeof(alphabet);
    }
}

/** Caller path/data may change after publication; every stored path, byte, span and UTF-8 byte column remains original. */
static void it_copies_borrowed_input_and_rejects_unrepresentable_length_before_allocation(void) {
    setup_test();
    char path[] = "original path";
    unsigned char bytes[] = {0xe4,0xb8,0xad,'\r','\n',0xf0,0x9f,0x98,0x80};
    const unsigned char expected[] = {0xe4,0xb8,0xad,'\r','\n',0xf0,0x9f,0x98,0x80};
    const SourceText *source = NULL;
    assert(source_manager_add(&sources, path, bytes, sizeof(bytes), &source) == SOURCE_OK);
    memset(path, 'x', sizeof(path) - 1);
    memset(bytes, 0, sizeof(bytes));
    assert(strcmp(source_path(source), "original path") == 0);
    assert(memcmp(source_bytes(source), expected, sizeof(expected)) == 0);
    SourcePosition position;
    assert(source_position(source, 9, &position) && position.line == 2 && position.column == 5);
    const SourceText *unchanged = source;
    size_t before = allocation_count;
    failure_at = before + 1;
    assert(source_manager_add(&sources, "limit", expected, SIZE_MAX, &unchanged) == SOURCE_LIMIT);
    assert(allocation_count == before && unchanged == source && sources.first == source);
    failure_at = 0;
    assert(source_manager_add(&sources, NULL, expected, 1, &unchanged) == SOURCE_INVALID_ARGUMENT);
    assert(source_manager_add(&sources, "invalid", NULL, 1, &unchanged) == SOURCE_INVALID_ARGUMENT);
    assert(source_manager_add(&sources, "invalid", expected, 1, NULL) == SOURCE_INVALID_ARGUMENT);
    assert(unchanged == source && allocation_count == before);
    destroy_test();
}
