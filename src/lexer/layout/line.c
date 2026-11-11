#include <string.h>
#include "internal.h"

void layout_classify(LayoutLine *line) {
    const unsigned char *bytes = source_bytes(line->physical.source);
    size_t start = line->physical.start;
    size_t end = line->physical.end;
    while (start < end && bytes[start] == ' ') {
        start++;
    }
    line->indentation = start - line->physical.start;
    line->kind = start == end ? LINE_BLANK : bytes[start] == '#' ? LINE_COMMENT
        : bytes[start] == '|' ? LINE_LITERAL : LINE_CODE;
    line->payload = (SourceSpan){line->physical.source,
        start < end && bytes[start] == '|' ? start + 1 : end, end};
    size_t code_end = start;
    while (code_end < end && bytes[code_end] != '#') {
        code_end++;
    }
    while (code_end > start && bytes[code_end - 1] == ' ') {
        code_end--;
    }
    line->code = (SourceSpan){line->physical.source, start, code_end};
}

bool layout_text_is(SourceSpan span, const char *text) {
    size_t length = strlen(text);
    return span.source != NULL && span.end >= span.start
        && span.end - span.start == length
        && memcmp(source_bytes(span.source) + span.start, text, length) == 0;
}
