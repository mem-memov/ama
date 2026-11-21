/**
 * @file
 * @brief Retain complete qualified constant operands before scoped path resolution.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) requires alias
 * expansion and normalization before member selection; syntax alone cannot split it.
 */
#include "internal.h"
/** Validate raw path syntax and retain its original interval without the list suffix.
 * An alias can supply the entire module/member identity, so no written slash or
 * minimum normalized component count can be required at this syntax boundary.
 * Component contract: [CONST-05](../../../../spec/language/declarations.md#const-05). */
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
    source->kind = AST_CONSTANT_SOURCE;
    source->qualified_path = path;
    return true;
}
