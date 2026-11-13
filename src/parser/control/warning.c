/**
 * @file
 * @brief Record empty written bodies before lowering, even if unreachable.
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) counts parsed
 * statements, including declarations, and requires one warning at each empty marker.
 */
#include "../internal.h"
bool parser_empty_warning(Parser *parser, AstBlock body, SourceSpan marker) {
    if (marker.source == NULL || body.first != NULL) { return true; }
    AstWarning *warning = parser_allocate(parser, sizeof(*warning), marker);
    if (warning == NULL) { return false; }
    warning->diagnostic = (Diagnostic){.code = DIAGNOSTIC_EMPTY_BODY,
        .message = "This written control body contains no statements",
        .primary = {.span = marker}};
    AstWarning **position = &parser->module->warnings;
    while (*position != NULL && (*position)->diagnostic.primary.span.start < marker.start) {
        position = &(*position)->next;
    }
    warning->next = *position;
    *position = warning;
    return true;
}
