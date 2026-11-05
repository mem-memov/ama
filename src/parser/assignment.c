#include "parser/internal.h"

bool parse_assignment(StatementParser* parser, uint32_t indentation, uint32_t destination_symbol_id) {
    Token value = token_stream_advance(parser->stream);
    if (value.indentation != indentation)
        return parser_error(parser, value.line_number,
            "assignment source must be at indent %u, got %u",
            indentation, value.indentation);
    return emit_parsed_value(parser, value, destination_symbol_id, "assignment source");
}
