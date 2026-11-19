/**
 * @file
 * @brief Parse ordinary section entries as variable-name/type pairs only.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps explicit
 * static definitions and applications in their separate graph.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) permits section
 * markers and source spelling, never symbol lookup, to select them.
 */
#include "../internal.h"

/** Parse one finite section interval in source order; partial records remain
 * AST-owned and no node is appended before its complete production succeeds.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * excludes standalone static declarations from ordinary interfaces.
 */
bool parser_declarations(Parser *parser, ParsedSection section, AstBlock *output) {
    parser->line = section.first;
    parser->end = section.end;
    parser_skip_region(parser, 0);
    while (parser->line != NULL && parser->line != parser->end) {
        AstNode *node = parser_node(parser, AST_DECLARATION, parser->line->code);
        if (node == NULL || parser->line->indentation != 0) {
            return node == NULL ? false : parser_fail(parser, parser_site(parser),
                "Section declarations start at indentation zero", EXPECT_TOP_DECLARATION);
        }
        if (!parser_declaration(parser, 0, node)) {
            return false;
        }
        parser_append(output, node);
        parser_skip_region(parser, 0);
    }
    return true;
}
