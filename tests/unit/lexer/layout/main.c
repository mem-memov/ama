#include "../common.c"

static void it_preserves_literal_bytes_trailing_spaces_comments_and_line_endings(void) {
    setup_test();
    LayoutStatus status = read_layout("@! # body\r\n |@C # data\t  \r\n # comment\n  ");
    assert(status == LAYOUT_OK);
    status = layout_validate(&layout, false);
    assert(status == LAYOUT_OK);
    LayoutLine *line = layout.first;
    assert(layout_text_is(line->code, "@!") && line->ending == ENDING_CRLF);
    line = line->next;
    assert(line->kind == LINE_LITERAL && line->indentation == 1);
    assert(layout_text_is(line->payload, "@C # data\t  "));
    line = line->next;
    assert(line->kind == LINE_COMMENT && line->ending == ENDING_LF);
    line = line->next;
    assert(line->kind == LINE_BLANK && line->ending == ENDING_NONE && line->next == NULL);
    destroy_layout();
}

static void it_separates_lenient_draft_reading_from_active_whitespace_validation(void) {
    const char *invalid[] = {"unfinished\tname", "x\ry", "\t|data"};
    for (size_t index = 0; index < 3; index++) {
        setup_test();
        LayoutStatus status = read_layout(invalid[index]);
        assert(status == LAYOUT_OK);
        status = layout_validate(&layout, false);
        assert(status == LAYOUT_INVALID && source_span_valid(layout.diagnostic.primary.span));
        destroy_layout();
    }
}

static void it_rejects_malformed_utf8_even_without_active_sections(void) {
    setup_test();
    LayoutStatus status = read_layout("draft\xff");
    assert(status == LAYOUT_INVALID && layout.diagnostic.code == DIAGNOSTIC_INVALID_UTF8);
    assert(layout.diagnostic.primary.span.start == 5);
    destroy_layout();
}

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

void run(void) {
    it_preserves_literal_bytes_trailing_spaces_comments_and_line_endings();
    it_separates_lenient_draft_reading_from_active_whitespace_validation();
    it_rejects_malformed_utf8_even_without_active_sections();
    it_cleans_partial_line_storage_after_each_allocation_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
