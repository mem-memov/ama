/**
 * @file
 * @brief Consume layout only within an explicitly supplied grammar region.
 * [SYN-02](../../../spec/language/source-and-names.md#syn-02) permits blank lines
 * but forbids comments from inventing a body or reopening it after a dedent.
 */
#include "../internal.h"
void parser_skip_region(Parser *parser, size_t indentation) {
    while (parser->line != NULL && parser->line != parser->end) {
        const LayoutLine *line = parser->line;
        if (line->kind != LINE_BLANK
            && (line->kind != LINE_COMMENT || line->indentation != indentation)) { break; }
        parser->line = line->next;
    }
}
