#ifndef AMA_PARSER_BINDINGS_INTERNAL_H
#define AMA_PARSER_BINDINGS_INTERNAL_H
#include "../../internal.h"
bool parser_binding(Parser *parser, AstArgument *argument);
bool parser_append_argument(Parser *parser, AstNode *node, AstArgument *argument);
#endif
