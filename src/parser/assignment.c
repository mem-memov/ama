#include "parser/internal.h"

bool parse_assignment(StatementParser* parser, uint32_t indentation, AstStatement* statement) {
    Token value = token_stream_advance(parser->stream);
    if (value.indentation != indentation)
        return parser_error(parser, value.line_number,
            "assignment source must be at indent %u, got %u", indentation, value.indentation);
    return parse_value(parser, value, &statement->as.assignment.source, "assignment source");
}
