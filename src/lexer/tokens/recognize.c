/**
 * @file
 * @brief Recognize original complete-name bytes and the longest fixed token at each position.
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) preserves single-space
 * Unicode names; [SYN-06](../../../spec/language/source-and-names.md#syn-06) fixes punctuation inventory.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"
/** Immutable spelling/category pair ordered longest first; equal-length alternatives have disjoint spellings. */
typedef struct { const char *text; LexKind kind; } FixedToken;

/** Scan one XID word, returning the unchanged offset when its first scalar is not a valid start. */
static size_t word(const unsigned char *bytes, size_t offset, size_t end) {
    size_t start = offset;
    while (offset < end) {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes + offset, end - offset, &scalar, &width)
            || !(offset == start ? unicode_identifier_start(scalar) : unicode_identifier_continue(scalar))) { break; }
        offset += width;
    }
    return offset;
}
bool lex_recognize(LexCursor *cursor, size_t offset, LexToken *token) {
    const unsigned char *bytes = source_bytes(cursor->line->code.source);
    size_t end = cursor->line->code.end;
    size_t stop = word(bytes, offset, end);
    if (stop != offset) {
        while (stop < end && bytes[stop] == ' ') {
            size_t next = word(bytes, stop + 1, end);
            if (next == stop + 1) { break; }
            stop = next;
        }
        *token = (LexToken){.kind = LEX_NAME, .text = {cursor->line->code.source, offset, stop}};
        return true;
    }
    static const FixedToken fixed[] = {
        {"&[]",LEX_REFERENCE_LIST}, {"@=",LEX_CONSTANTS}, {"@<",LEX_PARAMETERS},
        {"@>",LEX_RESULTS}, {"@!",LEX_AMA}, {"@C",LEX_NATIVE}, {">>",LEX_READ},
        {"[]",LEX_LIST}, {"&[",LEX_REFERENCE_INDEX}, {"..",LEX_STANDARD_ROOT},
        {".",LEX_DOT}, {"&",LEX_REFERENCE}, {"[",LEX_INDEX_OPEN}, {"]",LEX_INDEX_CLOSE},
        {"!",LEX_EXECUTE}, {"%",LEX_CONTINUATION}, {"<",LEX_INPUT}, {">",LEX_OUTPUT},
        {"?",LEX_QUESTION}, {":",LEX_COLON}, {"{",LEX_LOOP_OPEN}, {"}",LEX_LOOP_CLOSE}, {"^",LEX_BREAK}
    };
    for (size_t index = 0; index < sizeof(fixed) / sizeof(fixed[0]); index++) {
        size_t length = strlen(fixed[index].text);
        if (length <= end - offset && memcmp(bytes + offset, fixed[index].text, length) == 0) {
            *token = (LexToken){.kind = fixed[index].kind, .text = {cursor->line->code.source, offset, offset + length}};
            return true;
        }
    }
    if (bytes[offset] == '|' && (cursor->line->kind == LINE_LITERAL || cursor->line->kind == LINE_INPUT_LITERAL)
        && offset + 1 == end) {
        *token = (LexToken){.kind = LEX_LITERAL, .text = {cursor->line->code.source, offset, end}, .payload = cursor->line->payload};
        return true;
    }
    return lex_failure(cursor, offset, "Invalid code character or literal position", "a Unicode name or exact Ama punctuation token");
}
