/**
 * @file
 * @brief Check byte-accurate Unicode locations, source directives and failure-safe diagnostic output.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <stdio.h>
#include "../common.c"
#include "diagnostics/render/render.h"
#include "source/mapping/directive.h"

/**
 * Read the captured diagnostic stream into the fixture's bounded observation buffer after
 * successful rewind/IO checks.
 */
static void read_output(FILE *stream, char *bytes, size_t capacity) {
    int positioned = fseek(stream, 0, SEEK_SET);
    assert(positioned == 0);
    size_t count = fread(bytes, 1, capacity - 1, stream);
    assert(!ferror(stream) && count < capacity - 1);
    bytes[count] = 0;
}

#include "support/failures.c"
#include "support/contexts.c"

/**
 * Assert that the component renders original names byte coordinates and related declarations.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_renders_original_names_byte_coordinates_and_related_declarations(void) {
    setup_test();
    DiagnosticRelated related = {{{source, 10, 20}, NULL}, "required type", ".shop.item"};
    Diagnostic diagnostic = {DIAGNOSTIC_TYPE_MISMATCH, "Incompatible type", "商品",
        {{source, 3, 9}, NULL}, "Types differ", ".shop.item", NULL, &related, 1, NULL};
    FILE *stream = tmpfile();
    assert(stream != NULL);
    DiagnosticRenderStatus rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    char text[2048];
    read_output(stream, text, sizeof(text));
    const char *expected = "商店/item:2:1: error[type-mismatch]: Incompatible type\n"
        "    subject: 商品\n    2 | 商品\n    span: bytes [3, 9)\n"
        "    cause: Types differ\n    expected: .shop.item\n"
        "商店/item:3:1: note: required type\n    subject: .shop.item\n"
        "    3 | .shop.item\n    span: bytes [10, 20)\n";
    assert(strcmp(text, expected) == 0);
    int closed = fclose(stream);
    assert(closed == 0);
    destroy_test();
}

/**
 * Assert that the component handles multiline empty eof and malformed source excerpts. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_handles_multiline_empty_eof_and_malformed_source_excerpts(void) {
    setup_test();
    const unsigned char bytes[] = {'a', '\r', '\n', 0xff, '\t', '\n'};
    const SourceText *malformed;
    SourceStatus added = source_manager_add(&sources, "raw", bytes, sizeof(bytes), &malformed);
    assert(added == SOURCE_OK);
    FILE *stream = tmpfile();
    assert(stream != NULL);
    Diagnostic diagnostic = {0};
    diagnostic.code = DIAGNOSTIC_INVALID_UTF8;
    diagnostic.message = "Invalid input";
    diagnostic.primary.span = (SourceSpan){malformed, 0, sizeof(bytes)};
    DiagnosticRenderStatus rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    diagnostic.primary.span = (SourceSpan){malformed, sizeof(bytes), sizeof(bytes)};
    rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    char text[2048];
    read_output(stream, text, sizeof(text));
    assert(strstr(text, "1 | a\n    2 | \\xFF\\x09\n") != NULL);
    assert(strstr(text, "raw:3:1:") != NULL && strstr(text, "span: bytes [6, 6)") != NULL);
    int closed = fclose(stream);
    assert(closed == 0);
    destroy_test();
}

/**
 * Assert that the component rejects invalid records before output and reports output failures.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_invalid_records_before_output_and_reports_output_failures(void) {
    setup_test();
    FILE *stream = tmpfile();
    assert(stream != NULL);
    Diagnostic diagnostic = {0};
    DiagnosticRenderStatus rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_INVALID);
    long length = ftell(stream);
    assert(length == 0);
    int closed = fclose(stream);
    assert(closed == 0);
    stream = fopen("/dev/full", "wb");
    assert(stream != NULL);
    diagnostic.code = DIAGNOSTIC_OUT_OF_MEMORY;
    diagnostic.message = "Allocation failed";
    rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_IO_ERROR);
    (void)fclose(stream);
    destroy_test();
}

/**
 * Assert that the component escapes source mapping paths without rewriting unicode. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_escapes_source_mapping_paths_without_rewriting_unicode(void) {
    setup_test();
    const SourceText *mapped;
    SourceStatus status = source_manager_add(&sources, "商店/\"item\\name\t", NULL, 0, &mapped);
    assert(status == SOURCE_OK);
    FILE *stream = tmpfile();
    assert(stream != NULL);
    status = source_write_line_directive(stream, mapped, 0);
    assert(status == SOURCE_OK);
    char text[512];
    read_output(stream, text, sizeof(text));
    assert(strcmp(text, "#line 1 \"商店/\\\"item\\\\name\\011\"\n") == 0);
    long length = ftell(stream);
    status = source_write_line_directive(stream, mapped, 1);
    assert(status == SOURCE_INVALID_ARGUMENT);
    assert(ftell(stream) == length);
    int closed = fclose(stream);
    assert(closed == 0);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_renders_fallback_and_source_context_with_all_compiler_allocations_denied();
    it_renders_caller_contexts_after_original_notes_and_refuses_context_cycles_before_output();
    it_rejects_invalid_mapping_before_output_and_reports_immediate_io_failure();
    it_renders_original_names_byte_coordinates_and_related_declarations();
    it_handles_multiline_empty_eof_and_malformed_source_excerpts();
    it_rejects_invalid_records_before_output_and_reports_output_failures();
    it_escapes_source_mapping_paths_without_rewriting_unicode();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
