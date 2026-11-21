/**
 * @file
 * @brief Check original INI lines, global encoding refusals and pinned spacing.
 * The [configuration contract](../../../../../docs/implementation/project-configuration.md)
 * separates this allocation-free scanner from section/value interpretation.
 */
#include "../../common.c"
#include "source/configuration/lines/internal.h"

/** Install exact bounded bytes; caller observations outlive the borrowed cursor. */
static const SourceText *input(const unsigned char *bytes, size_t length) {
    const SourceText *source = NULL;
    assert(source_manager_add(&sources, "package.ini", bytes, length, &source) == SOURCE_OK);
    return source;
}

/** LF, CRLF and final unterminated records publish exact spans without phantom final lines. */
static void it_preserves_original_lines_and_all_physical_ending_forms(void) {
    const char *texts[] = {"", "\n", "\r\n", "a", "a\n", "a\r\n", ";a\n\n", "a\r\nb\nc"};
    const size_t counts[] = {0, 1, 1, 1, 1, 1, 2, 3};
    const size_t starts[][3] = {{0}, {0}, {0}, {0}, {0}, {0}, {0, 3}, {0, 3, 5}};
    const size_t ends[][3] = {{0}, {0}, {0}, {1}, {1}, {1}, {2, 3}, {1, 4, 6}};
    for (size_t example = 0; example < sizeof(texts) / sizeof(*texts); example++) {
        setup_test();
        const SourceText *source = input((const unsigned char *)texts[example], strlen(texts[example]));
        ConfigurationLines lines;
        size_t before = allocation_count;
        failure_at = allocation_count + 1;
        assert(configuration_lines_begin(&lines, source));
        for (size_t index = 0; index < counts[example]; index++) {
            SourceSpan span = {0};
            assert(configuration_line_next(&lines, &span) == CONFIGURATION_LINE);
            assert(span.source == source && span.start == starts[example][index]);
            assert(span.end == ends[example][index] && source_span_valid(span));
        }
        SourceSpan output = {source, 0, 0};
        unsigned char saved[sizeof(output)];
        memcpy(saved, &output, sizeof(output));
        assert(configuration_line_next(&lines, &output) == CONFIGURATION_END);
        assert(configuration_line_next(&lines, &output) == CONFIGURATION_END);
        assert(memcmp(saved, &output, sizeof(output)) == 0 && allocation_count == before);
        destroy_test();
    }
}

/** The scanner never treats semicolons, equals signs, hashes or NUL data as an implicit comment/value grammar. */
static void it_keeps_internal_payload_bytes_and_noninitial_bom_without_allocation(void) {
    static const unsigned char bytes[] = "~name = \xe5\x80\xbc;#=x\0y\xef\xbb\xbf";
    setup_test();
    const SourceText *source = input(bytes, sizeof(bytes) - 1);
    ConfigurationLines lines;
    assert(configuration_lines_begin(&lines, source));
    SourceSpan span = {0};
    assert(configuration_line_next(&lines, &span) == CONFIGURATION_LINE);
    assert(span.start == 0 && span.end == sizeof(bytes) - 1);
    assert(memcmp(source_bytes(source), bytes, sizeof(bytes) - 1) == 0);
    assert(configuration_line_next(&lines, &span) == CONFIGURATION_END);
    destroy_test();
}

/** Malformed bytes are rejected globally before any prefix line can be consumed. */
static void it_rejects_bom_tabs_lone_cr_and_malformed_utf8_at_original_offsets(void) {
    const char *invalid[] = {"\xef\xbb\xbf;a", ";a\n;b\t", "a\r", ";a\r;b", "a\n\xff", "a\n\xc0\xaf",
        "a\n\xed\xa0\x80", "a\n\xf4\x90\x80\x80", "a\n\xe2\x82"};
    const size_t offsets[] = {0, 5, 1, 2, 2, 2, 2, 2, 2};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        const SourceText *source = input((const unsigned char *)invalid[index], strlen(invalid[index]));
        ConfigurationLines lines;
        assert(!configuration_lines_begin(&lines, source));
        assert(lines.failed && lines.issue.span.source == source);
        assert(lines.issue.span.start == offsets[index] && source_span_valid(lines.issue.span));
        assert(lines.issue.message != NULL && lines.issue.expected != NULL);
        SourceSpan output = {0};
        unsigned char saved[sizeof(output)];
        memcpy(saved, &output, sizeof(output));
        assert(configuration_line_next(&lines, &output) == CONFIGURATION_INVALID_LINE);
        assert(memcmp(saved, &output, sizeof(output)) == 0);
        destroy_test();
    }
    ConfigurationLines lines;
    assert(!configuration_lines_begin(&lines, NULL));
    assert(lines.failed && lines.issue.span.source == NULL && lines.issue.span.start == 0);
}

/** Leading/trailing whitespace is refused even on comments; no trim or output publication can hide it. */
static void it_rejects_boundary_padding_and_spaces_only_lines_without_rewriting(void) {
    const char *invalid[] = {" ", "  ", " a", "a ", ";comment ", " ;comment", "a\n b", "a\n;b\xc2\xa0",
        "\xe3\x80\x80x", "x\xe2\x80\xa8"};
    const size_t offsets[] = {0, 0, 0, 1, 8, 0, 2, 4, 0, 1};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        const SourceText *source = input((const unsigned char *)invalid[index], strlen(invalid[index]));
        ConfigurationLines lines;
        assert(configuration_lines_begin(&lines, source));
        SourceSpan output = {0};
        ConfigurationLineStatus status;
        do {
            unsigned char saved[sizeof(output)];
            memcpy(saved, &output, sizeof(output));
            status = configuration_line_next(&lines, &output);
            if (status == CONFIGURATION_INVALID_LINE) { assert(memcmp(saved, &output, sizeof(output)) == 0); }
        } while (status == CONFIGURATION_LINE);
        assert(status == CONFIGURATION_INVALID_LINE && lines.issue.span.start == offsets[index]);
        assert(memcmp(source_bytes(source), invalid[index], strlen(invalid[index])) == 0);
        destroy_test();
    }
}

/** Exhaust the scalar domain against Unicode's independently enumerated 25 whitespace code points. */
static void it_matches_the_complete_pinned_whitespace_set_without_host_locale(void) {
    const uint32_t expected[] = {9, 10, 11, 12, 13, 32, 0x85, 0xa0, 0x1680, 0x2000, 0x2001,
        0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200a, 0x2028,
        0x2029, 0x202f, 0x205f, 0x3000};
    for (uint32_t scalar = 0; scalar <= 0x10ffff; scalar++) {
        bool listed = false;
        for (size_t index = 0; index < sizeof(expected) / sizeof(*expected); index++) {
            if (scalar == expected[index]) { listed = true; }
        }
        assert(configuration_space(scalar) == listed);
    }
    assert(!configuration_space(UINT32_MAX));
}

/** Run independent byte, span, failure-publication and complete property-domain checks. */
void run(void) {
    it_preserves_original_lines_and_all_physical_ending_forms();
    it_keeps_internal_payload_bytes_and_noninitial_bom_without_allocation();
    it_rejects_bom_tabs_lone_cr_and_malformed_utf8_at_original_offsets();
    it_rejects_boundary_padding_and_spaces_only_lines_without_rewriting();
    it_matches_the_complete_pinned_whitespace_set_without_host_locale();
}

/** Succeed only after all exact source observations complete and counted owners retire. */
int main(void) { run(); return EXIT_SUCCESS; }
