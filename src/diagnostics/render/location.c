/**
 * @file
 * @brief Render original byte coordinates and complete intersecting physical lines.
 *
 * [NAME-03](../../../spec/implementation/compiler.md#name-03)
 * Half-open spans include exactly their intersecting lines, with EOF/empty spans handled explicitly.
 */
#include "internal.h"

bool diagnostic_write_location(FILE *output, DiagnosticLocation location) {
    const char *path = location.span.source != NULL ? source_path(location.span.source)
        : location.path != NULL ? location.path : "<input>";
    if (!diagnostic_write_text(output, path)) {
        return false;
    }
    if (location.span.source != NULL) {
        SourcePosition position;
        bool found = source_position(location.span.source, location.span.start, &position);
        if (!found || fprintf(output, ":%zu:%zu", position.line, position.column) < 0) {
            return false;
        }
    }
    return true;
}

/** Render one already located physical line without line-ending bytes; preserve original content through byte escaping.
 */
static bool write_line(FILE *output, const SourceText *source, SourcePosition position) {
    return fprintf(output, "    %zu | ", position.line) >= 0
        && diagnostic_write_bytes(output, source_bytes(source) + position.line_start,
            position.line_end - position.line_start)
        && fputc('\n', output) != EOF;
}

bool diagnostic_write_excerpt(FILE *output, DiagnosticLocation location) {
    SourceSpan span = location.span;
    if (span.source == NULL) {
        return true;
    }
    SourcePosition first;
    SourcePosition last;
    bool located = source_position(span.source, span.start, &first)
        && source_position(span.source, span.end > span.start ? span.end - 1 : span.end, &last);
    if (!located) {
        return false;
    }
    SourcePosition position = first;
    for (;;) {
        if (!write_line(output, span.source, position)) {
            return false;
        }
        if (position.line == last.line) {
            break;
        }
        size_t next = position.line_end;
        const unsigned char *bytes = source_bytes(span.source);
        while (next < source_length(span.source) && bytes[next] != '\n') {
            next++;
        }
        if (next == source_length(span.source)
            || !source_position(span.source, next + 1, &position)) {
            return false;
        }
    }
    return fprintf(output, "    span: bytes [%zu, %zu)\n", span.start, span.end) >= 0;
}
