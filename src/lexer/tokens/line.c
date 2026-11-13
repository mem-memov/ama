/**
 * @file
 * @brief Check structural region indentation and nonliteral physical-line whitespace.
 * [SYN-02](../../../spec/language/source-and-names.md#syn-02) distinguishes trailing
 * comment prefixes from code spaces and requires exact comment-body indentation.
 */
#include "internal.h"
bool lex_failure(LexCursor *cursor, size_t offset, const char *message, const char *expected) {
    const SourceText *source = cursor->line == NULL ? NULL : cursor->line->physical.source;
    size_t end = source != NULL && offset < source_length(source) ? offset + 1 : offset;
    cursor->error = (Diagnostic){.code = DIAGNOSTIC_SYNTAX, .message = message,
        .primary = {.span = {source, offset, end}}, .expected = expected};
    cursor->failed = true;
    return false;
}
bool lex_line_begin(LexCursor *cursor, const LayoutLine *line, size_t indentation) {
    *cursor = (LexCursor){.line = line};
    if (line == NULL) { return lex_failure(cursor, 0, "A physical source line is required", "a retained layout line"); }
    cursor->offset = line->code.start;
    if (line->kind == LINE_BLANK) {
        return line->physical.start == line->physical.end
            || lex_failure(cursor, line->physical.start, "A blank line cannot contain spaces", "an empty physical line");
    }
    if (line->indentation != indentation) {
        return lex_failure(cursor, line->physical.start,
            "The line does not match its containing region's indentation", "exactly the containing region's ASCII-space indentation");
    }
    if (line->kind == LINE_LITERAL || line->kind == LINE_INPUT_LITERAL || line->kind == LINE_COMMENT) { return true; }
    const unsigned char *bytes = source_bytes(line->physical.source);
    size_t end = line->code.end;
    while (end < line->physical.end && bytes[end] == ' ') { end++; }
    if (end == line->physical.end && line->code.end != end) {
        return lex_failure(cursor, line->code.end, "Code has trailing spaces", "a line ending or trailing comment");
    }
    return true;
}
