/**
 * @file
 * @brief Compose section declarations with explicit static input/result source forms.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) makes a static
 * input formal and a defined static result different section productions.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) permits section
 * markers and source spelling, never symbol lookup, to select them.
 */
#include "../internal.h"

/** Parse one finite section interval in source order; partial records remain
 * AST-owned and no node is appended before its complete production succeeds.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * keeps static and runtime entries interleaved without merging their interfaces.
 */
bool parser_declarations(Parser *parser, ParsedSection section, AstBlock *output) {
    parser->line = section.first;
    parser->end = section.end;
    bool inputs = section.marker != NULL && layout_text_is(section.marker->code, "@<");
    bool results = section.marker != NULL && layout_text_is(section.marker->code, "@>");
    parser_skip_region(parser, 0);
    while (parser->line != NULL && parser->line != parser->end) {
        AstNode *node = parser_node(parser, AST_DECLARATION, parser->line->code);
        if (node == NULL || parser->line->indentation != 0) {
            return node == NULL ? false : parser_fail(parser, parser_site(parser),
                "Section declarations start at indentation zero", EXPECT_TOP_DECLARATION);
        }
        SourceSpan text = parser->line->code;
        bool dotted = text.start < text.end && source_bytes(text.source)[text.start] == '.';
        if (dotted && inputs) {
            node->kind = AST_STATIC_FORMAL;
            if (!parser_static_formal(parser, 0, &node->as.static_formal)) { return false; }
            node->span.end = node->as.static_formal->span.end;
        } else if (dotted && results) {
            if (!parser_static_result(parser, 0, node)) { return false; }
        } else if (!parser_declaration(parser, 0, node)) {
            return false;
        }
        parser_append(output, node);
        parser_skip_region(parser, 0);
    }
    return true;
}
