/**
 * @file
 * @brief Publish one completely recognized token after exact separator validation.
 * [SYN-06](../../../spec/language/source-and-names.md#syn-06) forbids repeated
 * separators and unknown punctuation while preserving permitted literal data.
 */
#include "internal.h"
LexStatus lex_token_next(LexCursor *cursor, LexToken *output) {
    if (cursor->failed) { return LEX_INVALID; }
    const LayoutLine *line = cursor->line;
    if (line->kind == LINE_BLANK || line->kind == LINE_COMMENT || cursor->offset == line->code.end) { return LEX_END; }
    const unsigned char *bytes = source_bytes(line->code.source);
    size_t offset = cursor->offset;
    bool separator = bytes[offset] == ' ';
    if (separator) {
        offset++;
        if (offset == line->code.end || bytes[offset] == ' ') {
            (void)lex_failure(cursor, offset, "A code separator is not exactly one space", "one ASCII space followed by a token");
            return LEX_INVALID;
        }
    }
    LexToken token;
    if (!lex_recognize(cursor, offset, &token)) { return LEX_INVALID; }
    token.separator = separator;
    bool reference_prefix;
    if (!lex_boundary(cursor, &token, &reference_prefix)) { return LEX_INVALID; }
    cursor->offset = token.text.end;
    cursor->previous = token;
    cursor->has_previous = true;
    cursor->reference_prefix = reference_prefix;
    *output = token;
    return LEX_TOKEN;
}
