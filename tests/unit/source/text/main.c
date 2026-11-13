/**
 * @file
 * @brief Check stable original bytes, line positions, spans and source ownership.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <stdio.h>
#include "../common.c"
#include "support/buffer.c"
#include "support/positions.c"
#include "support/loading.c"

/**
 * Assert that the component preserves bytes and spans across additions. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_preserves_bytes_and_spans_across_additions(void) {
    setup_test();
    const unsigned char bytes[] = {'a', 0, '\r', '\n', 0xe4, 0xb8, 0xad, '\n'};
    const SourceText *first = NULL;
    SourceStatus status = source_manager_add(&sources, "目录/file", bytes, sizeof(bytes), &first);
    assert(status == SOURCE_OK);
    SourceSpan span = {first, 4, 7};
    for (size_t index = 0; index < 100; index++) {
        const SourceText *next;
        status = source_manager_add(&sources, "next", NULL, 0, &next);
        assert(status == SOURCE_OK);
    }
    assert(source_span_valid(span));
    assert(strcmp(source_path(first), "目录/file") == 0);
    assert(memcmp(source_bytes(first), bytes, sizeof(bytes)) == 0);
    SourcePosition position;
    bool found = source_position(first, 7, &position);
    assert(found && position.line == 2 && position.column == 4);
    assert(position.line_start == 4 && position.line_end == 7);
    found = source_position(first, sizeof(bytes), &position);
    assert(found && position.line == 3 && position.column == 1);
    found = source_position(first, sizeof(bytes) + 1, &position);
    assert(!found);
    assert(!source_span_valid((SourceSpan){first, 7, 4}));
    destroy_test();
}

/**
 * Assert that the component matches every offset against a linear line model. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_matches_every_offset_against_a_linear_line_model(void) {
    setup_test();
    const unsigned char bytes[] = "a\r\nb\nc\r";
    const SourceText *source;
    SourceStatus status = source_manager_add(&sources, "lines", bytes, sizeof(bytes) - 1, &source);
    assert(status == SOURCE_OK);
    size_t line = 1;
    size_t start = 0;
    for (size_t offset = 0; offset < sizeof(bytes); offset++) {
        SourcePosition position;
        bool found = source_position(source, offset, &position);
        assert(found && position.line == line && position.column == offset - start + 1);
        if (bytes[offset] == '\n') {
            line++;
            start = offset + 1;
        }
    }
    destroy_test();
}

/**
 * Assert that the component unwinds each failed allocation without publishing. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_unwinds_each_failed_allocation_without_publishing(void) {
    for (size_t failure = 1; failure <= 4; failure++) {
        setup_test();
        failure_at = failure;
        const SourceText *output = NULL;
        SourceStatus status = source_manager_add(&sources, "file", (const unsigned char *)"a\n", 2, &output);
        assert(status == SOURCE_OUT_OF_MEMORY);
        assert(output == NULL && sources.first == NULL);
        destroy_test();
    }
}

/**
 * Assert that the component loads original file bytes and reports io failures. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_loads_original_file_bytes_and_reports_io_failures(void) {
    setup_test();
    FILE *stream = fopen("generated/input", "wb");
    assert(stream != NULL);
    size_t written = fwrite("one\r\ntwo", 1, 8, stream);
    int closed = fclose(stream);
    assert(written == 8 && closed == 0);
    const SourceText *source = NULL;
    SourceStatus status = source_manager_load(&sources, "generated/input", &source);
    assert(status == SOURCE_OK && source_length(source) == 8);
    assert(memcmp(source_bytes(source), "one\r\ntwo", 8) == 0);
    status = source_manager_load(&sources, "generated/missing", &source);
    assert(status == SOURCE_IO_ERROR);
    for (size_t failure = 1; failure <= 5; failure++) {
        allocation_count = 0;
        failure_at = failure;
        const SourceText *output = NULL;
        status = source_manager_load(&sources, "generated/input", &output);
        assert(status == SOURCE_OUT_OF_MEMORY && output == NULL);
    }
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_bounds_source_buffer_growth_and_saturates_without_an_exact_growth_tail();
    it_appends_aliasing_chunks_without_use_after_free_or_partial_commit();
    it_preserves_eof_and_every_small_physical_line_boundary_without_fabricated_bytes();
    it_copies_borrowed_input_and_rejects_unrepresentable_length_before_allocation();
    it_loads_empty_and_unterminated_files_as_exact_original_bytes();
    it_unwinds_every_multichunk_load_allocation_preserving_existing_sources();
    it_preserves_bytes_and_spans_across_additions();
    it_matches_every_offset_against_a_linear_line_model();
    it_unwinds_each_failed_allocation_without_publishing();
    it_loads_original_file_bytes_and_reports_io_failures();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
