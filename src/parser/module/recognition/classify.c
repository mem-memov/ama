/**
 * @file
 * @brief Check shared lexical rules and classify genuine activation without parsing unfinished declarations.
 * [SYN-04](../../../../spec/language/source-and-names.md#syn-04) keeps drafts
 * outside full parsing while preserving [SYN-06](../../../../spec/language/source-and-names.md#syn-06) spacing and annotation boundaries.
 */
#include "../internal.h"

/** Check every token before activation publication; an annotation occupies its own zero-indent line. Component contract: [SYN-04](../../../../spec/language/source-and-names.md#syn-04), [SYN-06](../../../../spec/language/source-and-names.md#syn-06). */
static bool check_line(const LayoutLine *line, bool *active, ParseError *error) {
    LexCursor cursor;
    if (!lex_line_begin(&cursor, line, line->indentation)) {
        parser_error_from_layout(error, &cursor.error);
        return false;
    }
    LexToken token;
    LexStatus status;
    size_t count = 0;
    while ((status = lex_token_next(&cursor, &token)) == LEX_TOKEN) {
        bool annotation = token.kind >= LEX_CONSTANTS && token.kind <= LEX_STATIC_MODULE;
        if (annotation && (count != 0 || line->indentation != 0 || token.text.end != line->code.end)) {
            return parser_error_set(error, token.text,
                "A section annotation occupies its own zero-indent line", EXPECT_SECTION_ORDER);
        }
        if (annotation && (token.kind == LEX_CONSTANTS || token.kind == LEX_AMA
            || token.kind == LEX_NATIVE || token.kind == LEX_STATIC_MODULE)) {
            *active = true;
        }
        count++;
    }
    if (status == LEX_INVALID) {
        parser_error_from_layout(error, &cursor.error);
        return false;
    }
    return true;
}
bool parser_module_classify(const SourceLayout *layout, bool *output, ParseError *error) {
    bool active = false;
    for (const LayoutLine *line = layout->first; line != NULL; line = line->next) {
        if (!check_line(line, &active, error)) { return false; }
    }
    *output = active;
    return true;
}
