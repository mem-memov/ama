#include "parser/internal.h"

bool parse_assign(CodeParser* P, uint32_t indent, uint32_t dst) {
    Token value = ts_next(P->stream);
    if (value.indent != indent)
        return parser_error(P, value.line,
                 "assignment source must be at indent %u, got %u",
                 indent, value.indent);
    return emit_parsed_value(P, value, dst, "assignment source");
}
