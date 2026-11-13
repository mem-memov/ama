/**
 * @file
 * @brief Recognize adjacent control markers through same-depth comments only.
 * [FLOW-01](../../../spec/language/control-flow.md#flow-01) excludes blank
 * lines and comments from other regions between a condition and its first marker.
 */
#include "../internal.h"
const LayoutLine *parser_condition_marker(const LayoutLine *line,
    const LayoutLine *end, size_t indentation) {
    while (line != NULL && line != end && line->kind == LINE_COMMENT
        && line->indentation == indentation) { line = line->next; }
    return line != NULL && line != end && line->kind != LINE_BLANK
        && line->indentation == indentation ? line : NULL;
}
