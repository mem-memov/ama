/**
 * @file
 * @brief Build explicit declaration/type/initializer structure in source order.
 *
 * [TYPE-01](../../../spec/language/declarations.md#type-01)
 * Do not infer a missing type or resolve a declaration name while parsing.
 */
#include "../internal.h"

/** Append one successfully parsed initializer, extend its parent span/count, then advance past layout. Partial storage remains AST-owned.
 */
static bool add_initializer(Parser *parser, AstNode *node, size_t indentation) {
    AstInitializer *initializer = parser_allocate(parser, sizeof(*initializer), parser_site(parser));
    if (initializer == NULL || !parser_source(parser, parser->line, parser->line->code,
            true, &initializer->source)) {
        return false;
    }
    if (node->as.declaration.last == NULL) {
        node->as.declaration.first = initializer;
    } else {
        node->as.declaration.last->next = initializer;
    }
    node->as.declaration.last = initializer;
    node->as.declaration.initializer_count++;
    node->span.end = initializer->source->span.end;
    parser->line = parser->line->next;
    parser_skip_region(parser, indentation);
    return true;
}

bool parser_declaration(Parser *parser, size_t indentation, AstNode *node) {
    node->kind = AST_DECLARATION;
    if (!parser_access(parser, parser->line->code, &node->as.declaration.name)) {
        return false;
    }
    if (!parser_declaration_name(node->as.declaration.name)) {
        return parser_fail(parser, node->span, "An indexed access is not a declaration name", EXPECT_DECLARATION_NAME);
    }
    parser->line = parser->line->next;
    parser_skip_region(parser, indentation);
    if (parser->line == NULL || parser->line == parser->end
        || parser->line->indentation != indentation || !parser_path(parser->line->code)) {
        return parser_fail(parser, parser_site(parser), "A declaration needs its module type on the next code line",
            EXPECT_TYPE_LINE);
    }
    node->as.declaration.type = parser->line->code;
    node->span.end = parser->line->code.end;
    parser->line = parser->line->next;
    parser_skip_region(parser, indentation + 1);
    while (parser->line != NULL && parser->line != parser->end
        && parser->line->indentation == indentation + 1) {
        if (!add_initializer(parser, node, indentation + 1)) {
            return false;
        }
    }
    return true;
}

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
    }
    return true;
}
