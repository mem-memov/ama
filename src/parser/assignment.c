#include "parser/internal.h"

void parse_assign(CodeParser* P, uint32_t indent, uint32_t dst) {
    Token value = ts_next(P->stream);
    if (value.indent != indent)
        fatal_at(P->module_path, value.line,
                 "assignment source must be at indent %u, got %u",
                 indent, value.indent);
    emit_parsed_value(P, value, dst, "assignment source");
}
