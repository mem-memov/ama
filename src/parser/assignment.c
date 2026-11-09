#include "parser/internal.h"

bool parse_assignment(StatementParser* parser, uint32_t indentation, AstStatement* statement) {
    Token value = token_stream_advance(parser->stream);
    if (value.type == TOKEN_END_OF_FILE)
        return parser_error(parser, statement->location.line,
            "expected assignment source or '!' after '%s'", statement->as.assignment.receiver.text);
    uint32_t expected = indentation + (parser->typed_declarations ? 1 : 0);
    if (value.indentation != expected)
        return parser_error(parser, value.line_number,
            "assignment source must be at indent %u, got %u", expected, value.indentation);
    return parse_value(parser, value, &statement->as.assignment.source, "assignment source");
}
