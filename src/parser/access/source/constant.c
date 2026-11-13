/**
 * @file
 * @brief Split complete qualified constant/list sources at their final component boundary.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) requires a real
 * module prefix and complete member; root dots are not member separators.
 */
#include "internal.h"
/** Split a validated retained qualified constant at its last dot, with explicit list suffix; require both module and member without resolving either.
 */
bool parser_constant_source(Parser *parser, SourceSpan text, AstSource *source) {
    const unsigned char *bytes = source_bytes(text.source);
    SourceSpan path = text;
    if (path.end >= path.start + 2 && bytes[path.end - 2] == '[' && bytes[path.end - 1] == ']') {
        source->constant_list = true;
        path.end -= 2;
    }
    if (!parser_path(path)) {
        return parser_fail(parser, text, "Invalid qualified constant source", EXPECT_CONSTANT_SOURCE);
    }
    size_t last_dot = path.end;
    while (last_dot > path.start && bytes[last_dot - 1] != '.') {
        last_dot--;
    }
    size_t root_width = path.end - path.start >= 2 && bytes[path.start + 1] == '.' ? 2 : 1;
    if (last_dot <= path.start + root_width) {
        return parser_fail(parser, text, "A constant source needs a module and member", EXPECT_QUALIFIED_CONSTANT);
    }
    source->kind = AST_CONSTANT_SOURCE;
    source->module_path = (SourceSpan){text.source, path.start, last_dot - 1};
    source->member = (SourceSpan){text.source, last_dot, path.end};
    return true;
}
