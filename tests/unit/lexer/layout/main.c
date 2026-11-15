/**
 * @file
 * @brief Check structural layout and literal byte boundaries with source lifetime and allocation failures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <stdio.h>
#include "../common.c"

/**
 * Assert that the component preserves literal bytes trailing spaces comments and line endings.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_preserves_literal_bytes_trailing_spaces_comments_and_line_endings(void) {
    setup_test();
    LayoutStatus status = read_layout("@! # body\r\n |@C # data  \r\n # comment\n");
    assert(status == LAYOUT_OK);
    LayoutLine *line = layout.first;
    assert(layout_text_is(line->code, "@!") && line->ending == ENDING_CRLF);
    assert(layout_text_is(line->comment, " # body"));
    line = line->next;
    assert(line->kind == LINE_LITERAL && line->indentation == 1);
    assert(layout_text_is(line->payload, "@C # data  "));
    assert(line->comment.source == NULL);
    line = line->next;
    assert(line->kind == LINE_COMMENT && line->ending == ENDING_LF);
    assert(layout_text_is(line->comment, " # comment"));
    line = line->next;
    assert(line == NULL);
    destroy_layout();
}

/**
 * Assert that the component preserves only exact input literal boundaries. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_preserves_only_exact_input_literal_boundaries(void) {
    const char *bindings[] = {" < |", " 消息 < |", " target& < |", " values[] < |", " values&[] < |"};
    for (size_t index = 0; index < sizeof(bindings) / sizeof(bindings[0]); index++) {
        setup_test();
        char text[160];
        int written = snprintf(text, sizeof(text), "%s# |@!  \r\n", bindings[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        LayoutStatus status = read_layout(text);
        assert(status == LAYOUT_OK);
        assert(layout.first->kind == LINE_INPUT_LITERAL);
        assert(layout_text_is(layout.first->payload, "# |@!  "));
        assert(layout.first->ending == ENDING_CRLF);
        destroy_layout();
    }
    const char *ordinary[] = {" <|text", " name< |text", " name & < |text", " > |text", " # < |text"};
    for (size_t index = 0; index < sizeof(ordinary) / sizeof(ordinary[0]); index++) {
        setup_test();
        LayoutStatus status = read_layout(ordinary[index]);
        assert(status == LAYOUT_OK && layout.first->kind != LINE_INPUT_LITERAL);
        destroy_layout();
    }
}

/**
 * Assert that global TAB and lone-CR checks precede draft or payload classification.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_global_whitespace_before_draft_or_payload_classification(void) {
    const char *invalid[] = {"unfinished\tname", "x\ry", "\t|data"};
    for (size_t index = 0; index < 3; index++) {
        setup_test();
        LayoutStatus status = read_layout(invalid[index]);
        assert(status == LAYOUT_INVALID && source_span_valid(layout.diagnostic.primary.span));
        destroy_layout();
    }
}

/**
 * Assert that the component rejects malformed utf8 even without active sections. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_rejects_malformed_utf8_even_without_active_sections(void) {
    setup_test();
    LayoutStatus status = read_layout("draft\xff");
    assert(status == LAYOUT_INVALID && layout.diagnostic.code == DIAGNOSTIC_INVALID_UTF8);
    assert(layout.diagnostic.primary.span.start == 5);
    destroy_layout();
}

/**
 * Assert that the component cleans partial line storage after each allocation failure. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_cleans_partial_line_storage_after_each_allocation_failure(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_test();
        const SourceText *source;
        SourceStatus added = source_manager_add(&sources, "source", (const unsigned char *)"a\nb\nc", 5, &source);
        assert(added == SOURCE_OK);
        allocation_count = 0;
        failure_at = failure;
        layout_init(&layout, sources.allocator);
        LayoutStatus status = layout_read(&layout, source);
        assert(status == LAYOUT_OUT_OF_MEMORY);
        destroy_layout();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_preserves_literal_bytes_trailing_spaces_comments_and_line_endings();
    it_preserves_only_exact_input_literal_boundaries();
    it_rejects_global_whitespace_before_draft_or_payload_classification();
    it_rejects_malformed_utf8_even_without_active_sections();
    it_cleans_partial_line_storage_after_each_allocation_failure();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
