#include <string.h>
#include "internal.h"

bool parser_fail(Parser *parser, SourceSpan span, const char *message, ParserExpectation expected) {
    if (!parser->failed) {
        parser_error_set(&parser->module->error, span, message, expected);
    }
    parser->failed = true;
    return false;
}

void *parser_allocate(Parser *parser, size_t size, SourceSpan site) {
    void *allocation = source_arena_allocate(&parser->module->arena, size);
    if (allocation == NULL) {
        parser_fail(parser, site, "Cannot allocate syntax tree storage", EXPECT_NONE);
        parser->module->error.diagnostic.code = DIAGNOSTIC_OUT_OF_MEMORY;
    } else {
        memset(allocation, 0, size);
    }
    return allocation;
}

SourceSpan parser_site(const Parser *parser) {
    if (parser->line != NULL && parser->line != parser->end) {
        return parser->line->code;
    }
    size_t end = parser->end != NULL ? parser->end->code.start
        : source_length(parser->module->layout.source);
    return (SourceSpan){parser->module->layout.source, end, end};
}

const LayoutLine *parser_significant(const LayoutLine *line, const LayoutLine *end, bool *blank) {
    while (line != NULL && line != end
        && (line->kind == LINE_BLANK || line->kind == LINE_COMMENT)) {
        *blank |= line->kind == LINE_BLANK;
        line = line->next;
    }
    return line;
}

void parser_skip_layout(Parser *parser) {
    bool blank = false;
    parser->line = parser_significant(parser->line, parser->end, &blank);
}

AstNode *parser_node(Parser *parser, AstNodeKind kind, SourceSpan span) {
    AstNode *node = parser_allocate(parser, sizeof(*node), span);
    if (node != NULL) {
        node->kind = kind;
        node->span = span;
    }
    return node;
}

void parser_append(AstBlock *block, AstNode *node) {
    if (block->last == NULL) {
        block->first = node;
    } else {
        block->last->next = node;
    }
    block->last = node;
}
