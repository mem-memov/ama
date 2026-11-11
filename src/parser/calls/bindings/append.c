#include "internal.h"

bool parser_append_argument(Parser *parser, AstNode *node, AstArgument *argument) {
    bool named = argument->label != NULL;
    if (node->as.call.first != NULL && named != node->as.call.named) {
        return parser_fail(parser, argument->span, "Named and positional bindings cannot mix", "one binding style throughout a call");
    }
    if (named && node->as.call.target != NULL && node->as.call.target->whole_list) {
        return parser_fail(parser, argument->span, "Whole-list bindings use the fixed positional interface",
            "positional <, > or >> bindings");
    }
    AstCallMode mode = node->as.call.mode;
    if ((mode == AST_ENTRY_CALL && argument->kind == AST_SAVED_OUTPUT)
        || (mode == AST_RESUME_CALL && argument->kind != AST_OUTPUT)
        || ((mode == AST_RETAINED_READ || mode == AST_CONSTANT_READ) && argument->kind != AST_SAVED_OUTPUT)
        || (argument->kind == AST_INPUT && node->as.call.output_count != 0)) {
        return parser_fail(parser, argument->marker, "Binding marker is not allowed in this call mode/order",
            mode == AST_ENTRY_CALL ? "inputs < before outputs >" : mode == AST_RESUME_CALL ? "outputs > only" : "retained outputs >> only");
    }
    node->as.call.named = named;
    argument->position = argument->kind == AST_INPUT
        ? node->as.call.input_count++ : node->as.call.output_count++;
    if (node->as.call.last == NULL) {
        node->as.call.first = argument;
    } else {
        node->as.call.last->next = argument;
    }
    node->as.call.last = argument;
    node->span.end = argument->span.end;
    return true;
}

