#ifndef AMA_PARSER_INTERNAL_H
#define AMA_PARSER_INTERNAL_H

#include "ast/ast.h"

typedef struct {
    AstModule *module;
    const LayoutLine *line;
    const LayoutLine *end;
    bool failed;
} Parser;

typedef enum { FRAME_ROOT, FRAME_BRANCH, FRAME_LOOP, FRAME_ALTERNATIVE } ParseFrameKind;
typedef struct ParseFrame {
    struct ParseFrame *parent;
    AstBlock *block;
    AstNode *owner;
    size_t indentation;
    ParseFrameKind kind;
} ParseFrame;

bool parser_fail(Parser *parser, SourceSpan span, const char *message, ParserExpectation expected);
void *parser_allocate(Parser *parser, size_t size, SourceSpan site);
SourceSpan parser_site(const Parser *parser);
const LayoutLine *parser_significant(const LayoutLine *line, const LayoutLine *end, bool *blank);
void parser_skip_layout(Parser *parser);
bool parser_name(SourceSpan text, size_t *cursor, SourceSpan *name);
bool parser_path(SourceSpan text);
bool parser_access(Parser *parser, SourceSpan text, AstAccess **output);
bool parser_declaration_name(const AstAccess *access);
bool parser_owned_access(const AstAccess *access);
bool parser_source(Parser *parser, const LayoutLine *line, SourceSpan text,
    bool literal_allowed, AstSource **output);
AstNode *parser_node(Parser *parser, AstNodeKind kind, SourceSpan span);
void parser_append(AstBlock *block, AstNode *node);
bool parser_declaration(Parser *parser, size_t indentation, AstNode *node);
bool parser_declarations(Parser *parser, ParsedSection section, AstBlock *output);
bool parser_body(Parser *parser, ParsedSection section, AstBlock *output);
bool parser_statement(Parser *parser, ParseFrame **frame);
bool parser_start_call(Parser *parser, ParseFrame **frame, AstNode *node,
    SourceSpan target, AstCallMode mode);
bool parser_call_arguments(Parser *parser, AstNode *node, size_t indentation);
bool parser_push_frame(Parser *parser, ParseFrame **frame, AstBlock *block,
    AstNode *owner, size_t indentation, ParseFrameKind kind);

#endif
