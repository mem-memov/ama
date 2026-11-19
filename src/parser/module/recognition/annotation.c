/**
 * @file
 * @brief Decode already checked section tokens into the grammar's fixed section order.
 * [SYN-03](../../../../spec/language/source-and-names.md#syn-03) defines exactly
 * ordinary sections and the exclusive static marker; no lookup determines kind.
 */
#include "../internal.h"
int parser_annotation(const LayoutLine *line) {
    if (line->kind != LINE_CODE || line->indentation != 0) { return -1; }
    LexCursor cursor;
    if (!lex_line_begin(&cursor, line, 0)) { return -1; }
    LexToken token;
    if (lex_token_next(&cursor, &token) != LEX_TOKEN || token.text.end != line->code.end) { return -1; }
    switch (token.kind) {
        case LEX_CONSTANTS: return 0;
        case LEX_PARAMETERS: return 1;
        case LEX_RESULTS: return 2;
        case LEX_AMA: return 3;
        case LEX_NATIVE: return 4;
        case LEX_STATIC_MODULE: return 5;
        default: return -1;
    }
}
