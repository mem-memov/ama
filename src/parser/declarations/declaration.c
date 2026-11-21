/**
 * @file
 * @brief Build explicit declaration/type/initializer structure in source order.
 *
 * [TYPE-01](../../../spec/language/declarations.md#type-01)
 * Do not infer a missing type or resolve a declaration name while parsing.
 * [TPL-15](../../../spec/language/templates.md#tpl-15) gives each declaration
 * a complete structural Type expression before its initializer list.
 */
#include "../internal.h"

/** Append one successfully parsed initializer, extend its parent span/count, then advance past layout. Partial storage remains AST-owned.
 * Component contract: [TYPE-01](../../../spec/language/declarations.md#type-01). */
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
        || parser->line->indentation != indentation
        || (!parser_path(parser->line->code) && !parser_static_name(parser->line->code))) {
        return parser_fail(parser, parser_site(parser), "A declaration needs its module type on the next code line",
            EXPECT_TYPE_LINE);
    }
    if (!parser_type_expression(parser, parser->line->code, indentation,
        PARSER_TYPE_DECLARATION, &node->as.declaration.type)) {
        return false;
    }
    node->span.end = node->as.declaration.type->span.end;
    parser_skip_region(parser, indentation + 1);
    while (parser->line != NULL && parser->line != parser->end
        && parser->line->indentation == indentation + 1) {
        if (!add_initializer(parser, node, indentation + 1)) {
            return false;
        }
    }
    return true;
}
